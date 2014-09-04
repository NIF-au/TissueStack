#include "tissuestack.h"
#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackTaskQueue::TissueStackTaskQueue()
{
	if (!tissuestack::utils::System::directoryExists(TASKS_PATH))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Tasks Directory does NOT exist!");

	// read in all task files listed in the queue file
	const std::vector<std::string> tasksIds = this->getTasksFromQueueFile();

	// walk through the list and build the task objects based on the task file content
	// this makes sure that we don't modify the tasks while we are populating
	std::lock_guard<std::mutex> lock(this->_tasks_mutex);
	for (auto id : tasksIds)
		this->buildTaskFromIndividualTaskFile(id);
}

tissuestack::services::TissueStackTaskQueue::~TissueStackTaskQueue() {}

void tissuestack::services::TissueStackTaskQueue::purgeInstance()
{
	// before we destroy everything, persist it to disk
	this->writeTasksToQueueFile();

	// this makes sure that we don't modify the tasks while we are doing this traversal
	std::lock_guard<std::mutex> lock(this->_tasks_mutex);

	// walk through entries, persist their state and clean them up
	for (auto task : this->_tasks)
	{
		if (task == nullptr)
			continue;

		this->writeBackToIndividualTasksFile(task);
		delete task;
	}
	this->_tasks.clear();

	delete tissuestack::services::TissueStackTaskQueue::_instance;
	tissuestack::services::TissueStackTaskQueue::_instance = nullptr;
}

tissuestack::services::TissueStackTaskQueue * tissuestack::services::TissueStackTaskQueue::instance()
{
	if (tissuestack::services::TissueStackTaskQueue::_instance == nullptr)
		tissuestack::services::TissueStackTaskQueue::_instance = new tissuestack::services::TissueStackTaskQueue();

	return tissuestack::services::TissueStackTaskQueue::_instance;
}

void tissuestack::services::TissueStackTaskQueue::writeBackToIndividualTasksFile(const tissuestack::services::TissueStackTask * task)
{
	if (task == nullptr) return;

	const std::string taskFile = std::string(TASKS_PATH) + "/" + task->getId();
	int fd = open(taskFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <= 0)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Could not persist individual task file %s\n", task->getId().c_str());
		return;
	}

	std::ostringstream taskFileContents;
	taskFileContents << task->getInputImageData()->getFileName() << std::endl; // line 1 in_file
	taskFileContents << task->getType() << std::endl; // line 2 type
	std::string params = "";

	// line 3 => params, we have to cast for that one
	if (task->getType() == tissuestack::services::TissueStackTaskType::CONVERSION)
		params =
			(static_cast<const tissuestack::services::TissueStackConversionTask *>(task))->getOutFile();
	else // TILING
		params =
			(static_cast<const tissuestack::services::TissueStackTilingTask *>(task))->getParametersForTaskFile();

	taskFileContents << params << std::endl; // line 3 params
	taskFileContents << task->getSlicesDone() << std::endl; // line 4 progress
	taskFileContents << task->getTotalSlices() << std::endl; // line 5 total work

	const std::string contents = taskFileContents.str();
	// write to file
	if (write(fd, contents.c_str(), contents.size()) < 0)
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Could not write into individual task file %s\n", task->getId().c_str());
	close(fd);
}

void tissuestack::services::TissueStackTaskQueue::writeTasksToQueueFile()
{
	std::ostringstream taskFileContents;
	// this makes sure that we don't modify the tasks while we are doing this traversal
	{
		std::lock_guard<std::mutex> lock(this->_tasks_mutex);

		taskFileContents << "";
		for (auto task : this->_tasks)
			if (task->getStatus() == tissuestack::services::TissueStackTaskStatus::QUEUED ||
					task->getStatus() == tissuestack::services::TissueStackTaskStatus::IN_PROCESS)
				taskFileContents << task->getId() << std::endl;
	}

	// this makes sure that we don't overwrite our queue file simultaneously
	std::lock_guard<std::mutex> lock(this->_queue_mutex);

	const std::string taskFile = std::string(TASKS_PATH) + "/general";
	// rename old task file
	if (rename(taskFile.c_str(), (taskFile + ".old").c_str()) != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failed to rename task queue file!");

	// create and open new task file for writing
	int fd = open(taskFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <= 0)
	{
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not create new task queue file!");
		rename((taskFile + ".old").c_str(), taskFile.c_str());
		return;
	}

	const std::string contents = taskFileContents.str();
	// dump new contents
	if (write(fd, contents.c_str(), contents.size()) < 0)
	{
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not save new task queue file!");
		unlink(taskFile.c_str());
		rename((taskFile + ".old").c_str(), taskFile.c_str());
		return;
	}
	// close file and delete old one
	close(fd);
	unlink((taskFile + ".old").c_str());
}

const bool tissuestack::services::TissueStackTaskQueue::isBeingTiled(const std::string in_file)
{
	if (in_file.empty() || !tissuestack::utils::System::fileExists(in_file)) return false;

	// this makes sure that we don't modify the tasks while we are doing this traversal
	std::lock_guard<std::mutex> lock(this->_tasks_mutex);

	for (auto t : this->_tasks)
	{
		if ((t->getInputImageData()->getFileName().compare(in_file) == 0)
				&& t->getType() == tissuestack::services::TissueStackTaskType::TILING
				&& (t->getStatus() == tissuestack::services::TissueStackTaskStatus::QUEUED ||
						t->getStatus() == tissuestack::services::TissueStackTaskStatus::IN_PROCESS))
			return true;
	}

	return false;
}

const std::string tissuestack::services::TissueStackTaskQueue::generateTaskId()
{
	// we use the present time as millis and append 3 digits of pseudo randomness
	std::string id = std::to_string(tissuestack::utils::System::getSystemTimeInMillis())
		+ tissuestack::utils::System::generatePseudoRandomNumberAsString(3);
	if (this->findTaskById(id)) // through some craziness this id exists, try again
		id =  std::to_string(tissuestack::utils::System::getSystemTimeInMillis())
				+ tissuestack::utils::System::generatePseudoRandomNumberAsString(3);

	return id;
}

const tissuestack::services::TissueStackTask * tissuestack::services::TissueStackTaskQueue::findTaskById(const std::string & id)
{
	std::lock_guard<std::mutex> lock(this->_tasks_mutex);

	for (auto t : this->_tasks)
		if (t->getId().compare(id) == 0)
			return t;

	return nullptr;
}

const bool tissuestack::services::TissueStackTaskQueue::isBeingConverted(const std::string in_file)
{
	if (in_file.empty() || !tissuestack::utils::System::fileExists(in_file)) return false;

	// this makes sure that we don't modify the tasks while we are doing this traversal
	std::lock_guard<std::mutex> lock(this->_tasks_mutex);

	for (auto t : this->_tasks)
	{
		if ((t->getInputImageData()->getFileName().compare(in_file) == 0)
				&& t->getType() == tissuestack::services::TissueStackTaskType::CONVERSION
				&& (t->getStatus() == tissuestack::services::TissueStackTaskStatus::QUEUED ||
						t->getStatus() == tissuestack::services::TissueStackTaskStatus::IN_PROCESS))
			return true;
	}

	return false;
}

inline void tissuestack::services::TissueStackTaskQueue::buildTaskFromIndividualTaskFile(const std::string & task_id)
{
	const std::vector<std::string> lines =
			tissuestack::utils::System::readTextFileLineByLine(std::string(TASKS_PATH) + "/" + task_id);

	/*
	 * Layout for task files is as follows:
	 * -------------------------------------
	 * 1. line: in file
	 * 2. line: enum value of type denoting either tiling or conversion
	 * 3. line:
	 * 		for conversion: the out file location
	 * 		for tiling: the following information all separated by |
	 *			tiling directory, dimension names (separated by ,)
	 *			zoom factors (separated by ,), color map name,
	 *			square length and image format
	 * 4. line:
	 *		'slice number' determining present progress uniquely
	 *		given the params above and the order of work/dimensions
	 * 5. line:
	 *		'total slice number' as determined by the params above
	 *
	 * All of the information is needed and if missing we'll make a note of that in the error log
	 */
	if (lines.size() != 5)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Task File %s does not have 5 lines!\n", task_id.c_str());
		return;
	}

	unsigned short lineNumber = 1;
	unsigned short type = 666;
	std::string in_file = "";
	std::string params = "";

	unsigned long long int sliceProgress = 0;
	unsigned long long int totalSlices = 0;
	for (auto l : lines)
	{
		switch (lineNumber)
		{
			case 1:
				in_file = l;
				break;
			case 2:
				type = static_cast<unsigned short>(atoi(l.c_str()));
				break;
			case 3:
				params = l;
				break;
			case 4:
				sliceProgress = strtoull(l.c_str(), NULL, 10);
				break;
			case 5:
				totalSlices = strtoull(l.c_str(), NULL, 10);
				break;
		}
		lineNumber++;
	}

	// some checks
	if (type != tissuestack::services::TissueStackTaskType::CONVERSION ||
			type != tissuestack::services::TissueStackTaskType::TILING)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Task File %s is neither conversion, nor tiling!\n", task_id.c_str());
		return;
	}

	if (sliceProgress > totalSlices) // we cannot have more progress than total work...
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Task File %s is neither conversion, nor tiling!\n", task_id.c_str());
		return;
	}
	if (sliceProgress == totalSlices) // we are finished and need not be added any more...
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Task File %s is neither conversion, nor tiling!\n", task_id.c_str());
		return;
	}

	// now try and instantiate the tasks, something can happen here still
	// we try catch and make a note and skip !!
	try
	{
		tissuestack::services::TissueStackTask * aNewTask = nullptr;

		if (type != tissuestack::services::TissueStackTaskType::CONVERSION)
		{
			aNewTask = new tissuestack::services::TissueStackConversionTask(
				task_id,
				in_file,
				params);
		} else if (type != tissuestack::services::TissueStackTaskType::CONVERSION)
		{
			// TODO: dissect params before we can go on

		}

		if (aNewTask)
			this->_tasks.push_back(aNewTask);
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to instantiate task %s: %s\n", task_id.c_str(), bad.what());
	}
}

const std::vector<std::string> tissuestack::services::TissueStackTaskQueue::getTasksFromQueueFile()
{
	// this makes sure that we don't read/write to the queue file at the same time!
	std::lock_guard<std::mutex> lock(this->_queue_mutex);

	const std::string taskFile = std::string(TASKS_PATH) + "/general";
	if (!tissuestack::utils::System::fileExists(taskFile))
	{
		// nothing there. perhaps we have an .old file from an interruption during queue file write
		if (tissuestack::utils::System::fileExists((taskFile + ".old").c_str()))
		{
			// hooray, let's restore it
			if (rename((taskFile + ".old").c_str(), taskFile.c_str()) == 0)
				return	tissuestack::utils::System::readTextFileLineByLine(taskFile);

			tissuestack::logging::TissueStackLogger::instance()->error("Failed to restore old Task Queue!\n");
		}
		tissuestack::logging::TissueStackLogger::instance()->error("Task Queue File does not exist! We'll try and create it!\n");
		int touchedFile = open(taskFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (touchedFile <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not create the tasks queue file!");
		close(touchedFile);
	}

	return tissuestack::utils::System::readTextFileLineByLine(taskFile);
}


 void tissuestack::services::TissueStackTaskQueue::dumpAllTasksToDebugLog() const
 {
	for (auto task : this->_tasks)
		task->dumpTaskToDebugLog();
 }

tissuestack::services::TissueStackTaskQueue * tissuestack::services::TissueStackTaskQueue::_instance = nullptr;
