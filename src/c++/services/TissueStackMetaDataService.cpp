/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

const std::string tissuestack::services::TissueStackMetaDataService::SUB_SERVICE_ID = "METADATA";



tissuestack::services::TissueStackMetaDataService::TissueStackMetaDataService() {
	this->addMandatoryParametersForRequest("DATASET_LIST",
		std::vector<std::string>{ "SESSION" });
	this->addMandatoryParametersForRequest("DATASET_MODIFY",
		std::vector<std::string>{ "SESSION", "ID", "COLUMN", "VALUE" });
	this->addMandatoryParametersForRequest("LIST_TASKS",
		std::vector<std::string>{ "SESSION", "TYPE", "STATUS" });
	this->addMandatoryParametersForRequest("TASK_STATUS",
		std::vector<std::string>{ "SESSION", "TYPE", "TASK_ID" });
};

tissuestack::services::TissueStackMetaDataService::~TissueStackMetaDataService() {};

void tissuestack::services::TissueStackMetaDataService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::TissueStackMetaDataService::streamResponse(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::string json = tissuestack::common::NO_RESULTS_JSON;

	// the following resources need a valid session
	if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
		request->getRequestParameter("SESSION")))
	{
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Invalid Session! Please Log In.");
	}

	if (action.compare("DATASET_LIST") == 0)
		json = this->handleDataSetListRequest(request);
	else if (action.compare("DATASET_MODIFY") == 0)
		json = this->handleDataSetModifyRequest(request);
	else if (action.compare("LIST_TASKS") == 0)
		json = this->handleTaskListingRequest(request);
	else if (action.compare("TASK_STATUS") == 0)
		json = this->handleTaskStatusRequest(request);

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json);
	write(file_descriptor, response.c_str(), response.length());
}

const std::string tissuestack::services::TissueStackMetaDataService::handleDataSetListRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const {

	const std::vector<const tissuestack::database::DataSetInfo *> results =
		tissuestack::database::MetaDataProvider::queryAllDataSets();

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto ds : results)
	{
		if (i !=0)
				json << ",";

		json << ds->getJson();
		i++;
		delete ds;
	}
	json << "]}";

	return json.str();
}

const std::string tissuestack::services::TissueStackMetaDataService::handleDataSetModifyRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const {

	const bool wasSuccessful =
		tissuestack::database::MetaDataProvider::updateDataSetInfo(
			strtoull(request->getRequestParameter("ID").c_str(), NULL, 10),
			request->getRequestParameter("COLUMN"),
			request->getRequestParameter("VALUE"));

	std::ostringstream json;
	json << "{\"response\": ";

	if (wasSuccessful)
		json << "\"modified dataset successfully\"";
	else
		json << "\"failed to modify dataset\"";
	json << "}";
	return json.str();
}

const std::string tissuestack::services::TissueStackMetaDataService::handleTaskListingRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const {

	const std::string status =
		request->getRequestParameter("STATUS", true);
	const std::string type =
		request->getRequestParameter("TYPE", true);
	if (type.compare("TILING") != 0 && type.compare("CONVERSION") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task type has to be 'TILING' or 'CONVERSION'!");

	const std::vector<std::string> filteredFiles = this->filterTasksDirectory(status);

	std::ostringstream json;
	json << "{\"response\": [";

	int counter = 0;
	for (auto f : filteredFiles) {
		const std::string resp = this->getTaskStatus(f, type);
		if (!resp.empty()) {
			if (counter != 0) json << ",";
			json << resp;
			counter++;
		}
	}

	json << "]}";
	return json.str();
}

const std::string tissuestack::services::TissueStackMetaDataService::handleTaskStatusRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const {
	const std::string task_id =
		request->getRequestParameter("TASK_ID");

	const std::string task_type =
		request->getRequestParameter("TYPE", true);
	if (task_type.compare("TILING") != 0 && task_type.compare("CONVERSION") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task type has to be 'TILING' or 'CONVERSION'!");

	const std::string resp = this->getTaskStatus(task_id, task_type, true);
	std::ostringstream json;
	json << "{\"response\": ";
	json << (resp.empty() ?
		"\"given task type does not match actual task type, e.g. supposed tiling task could be conversion\"" :
			resp);
	json << "}";
	return json.str();
}

inline const std::string tissuestack::services::TissueStackMetaDataService::getTaskStatus(
		const std::string & task_id, const std::string & task_type, const bool pure_task_id) const {

	std::string taskFile = task_id;
	if (!pure_task_id) { // for files with endings
		std::size_t dotPos = taskFile.find(".");
		std::size_t slashPos = taskFile.find_last_of("/");
		if (dotPos != std::string::npos && dotPos > 0) {
			taskFile = taskFile.substr(slashPos+1,dotPos-slashPos-1);
		} else
			taskFile = taskFile.substr(slashPos+1);
	}

	if (!tissuestack::utils::Misc::isNumber(taskFile))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task file name can be numeric only!");

	// check in memory first
	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(taskFile);
	if (hit) {
		// good we don't need to fish in the file
		if ((task_type.compare("TILING") == 0 &&
				hit->getType() == tissuestack::services::TissueStackTaskType::CONVERSION) ||
			(task_type.compare("CONVERSION") == 0 &&
				hit->getType() == tissuestack::services::TissueStackTaskType::TILING))
			return "";

		std::ostringstream json;
		json << "{\"task_id\": ";
		json << taskFile;
		json << ", \"filename\": \"";
		json << hit->getInputFileName();
		json << "\", \"progress\": ";
		json << hit->getProgress();
		json << ", \"status\": \"";
		switch (hit->getStatus()) {
			case tissuestack::services::TissueStackTaskStatus::QUEUED:
				json << "queued";
				break;
			case tissuestack::services::TissueStackTaskStatus::IN_PROCESS:
				json << "running";
				break;
			case tissuestack::services::TissueStackTaskStatus::ERRONEOUS:
				json << "failed";
				break;
			case tissuestack::services::TissueStackTaskStatus::FINISHED:
				json << "done";
				break;
			case tissuestack::services::TissueStackTaskStatus::UNZIPPING:
				json << "unzipping";
				break;
			case tissuestack::services::TissueStackTaskStatus::CANCELLED:
				json << "cancelled";
				break;
			default:
				json << "unknown";
		}
		json << "\"}";
		return json.str();
	} else {
		// for files that fell through above but have come in as pure task ids
		// we need to test for file endings ...
		std::string actualTaskId = task_id;
		if (pure_task_id) {
			actualTaskId = tissuestack::services::TissueStackTaskQueue::getTasksDirectory() + "/" + actualTaskId;
			if (tissuestack::utils::System::fileExists(std::string(actualTaskId + ".done")))
				actualTaskId += ".done";
			else if (tissuestack::utils::System::fileExists(std::string(actualTaskId + ".error")))
				actualTaskId += ".error";
			else if (tissuestack::utils::System::fileExists(std::string(actualTaskId + ".cancelled")))
				actualTaskId += ".cancelled";
		}

		// we have to go and look for the file....
		return this->getStatusFromTaskFile(actualTaskId, task_type);
	}

	THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");
}

inline const std::string tissuestack::services::TissueStackMetaDataService::getStatusFromTaskFile(
		const std::string & task_id,
		const std::string & task_type) const
{
	// existence check
	if (task_id.empty() || !tissuestack::utils::System::fileExists(task_id))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task file does not exist!");

	const std::vector<std::string> lines =
			tissuestack::utils::System::readTextFileLineByLine(task_id);

	/*
	 * Layout for task files is as follows:
	 * -------------------------------------
	 * 1. line: in file
	 * 2. line: enum value of type denoting either tiling or conversion
	 * 3. line:
	 * 		for conversion: the out file location
	 * 		for tiling: the following information all separated by |
	 *			tiling directory, dimension names (separated by ,)
	 *			zoom levels (separated by ,), color map name,
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task file seems to be corrupt or in a mid write state!");

	unsigned short lineNumber = 1;
	unsigned short type = 666;
	std::string in_file = "";

	unsigned long long int sliceProgress = 0;
	unsigned long long int totalSlices = 0;
	for (auto l : lines)
	{
		l = tissuestack::utils::Misc::eraseCharacterFromString(l, '\n');
		switch (lineNumber)
		{
			case 1:
				in_file = l;
				break;
			case 2:
				type = static_cast<unsigned short>(atoi(l.c_str()));
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

	// preliminary check
	if (type != tissuestack::services::TissueStackTaskType::CONVERSION &&
			type != tissuestack::services::TissueStackTaskType::TILING)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task file is neither TILING nor CONVERSION!");

	if ((task_type.compare("TILING") == 0 &&
			type == tissuestack::services::TissueStackTaskType::CONVERSION) ||
		(task_type.compare("CONVERSION") == 0 &&
			type == tissuestack::services::TissueStackTaskType::TILING))
		return "";

	// get the progress/total figures
	if (sliceProgress > totalSlices) // we cannot have more progress than total work...
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Erroneous Progress values!");

	float progress = 0;
	if (totalSlices != 0) {
		progress = (static_cast<float>(sliceProgress) / static_cast<float>(totalSlices)) *
			static_cast<float>(100);
	}

	std::string id = task_id;
	std::size_t dotPos = id.find(".");
	std::size_t slashPos = id.find_last_of("/");
	if (dotPos != std::string::npos && dotPos > 0) {
		id = id.substr(slashPos+1,dotPos-slashPos-1);
	} else
		id = id.substr(slashPos+1);

	std::ostringstream json;
	json << "{\"task_id\": ";
	json << id;
	json << ", \"filename\": \"";
	json << in_file;
	json << "\", \"progress\": ";
	json << std::to_string(progress);
	json << ", \"status\": \"";
	if (task_id.find(".done") != std::string::npos)
		json << "done";
	else if (task_id.find(".error") != std::string::npos)
		json << "error";
	else if (task_id.find(".cancelled") != std::string::npos)
		json << "cancelled";
	else if (progress == 100)
		json << "done";
	else json << "active";
	json << "\"}";
	return json.str();
}

const std::vector<std::string> tissuestack::services::TissueStackMetaDataService::filterTasksDirectory(const std::string status) const
{

	if (status.compare("ALL") != 0 && status.compare("DONE") != 0 && status.compare("CANCELLED") != 0
			&& status.compare("ERROR") != 0 && status.compare("ACTIVE") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"task filter for statuses can only take on values: ALL, DONE, ACTIVE, CANCELLED and ERROR!");

	std::vector<std::string> results;

	const std::string dir = tissuestack::services::TissueStackTaskQueue::getTasksDirectory();
	const std::vector<std::string> filesInUploadDirectory =
		tissuestack::utils::System::getFilesInDirectory(dir);

	for (auto f : filesInUploadDirectory)
	{
		// if file is 'general', we'll ignore it.
		if (f.find("general") != std::string::npos) continue;

		// numeric task number check
		// we have to chop off ending
		std::size_t dotPos = f.find(".");
		std::size_t slashPos = f.find_last_of("/");
		std::string potentialTaskFile = f;
		if (dotPos != std::string::npos && dotPos > 0) {
			potentialTaskFile = potentialTaskFile.substr(slashPos + 1,dotPos-slashPos-1);
		} else
			potentialTaskFile = potentialTaskFile.substr(slashPos + 1);

		if (!tissuestack::utils::Misc::isNumber(potentialTaskFile))
			continue; // we are not a number

		if (status.compare("ALL") == 0) {
			results.push_back(f);
			continue;
		}

		// status: done
		if (status.compare("DONE") == 0 && f.find(".done") != std::string::npos) {
			results.push_back(f);
			continue;
		}

		// status: cancelled
		if (status.compare("CANCELLED") == 0 && f.find(".cancelled") != std::string::npos) {
			results.push_back(f);
			continue;
		}

		// status: error
		if (status.compare("ERROR") == 0 && f.find(".error") != std::string::npos) {
			results.push_back(f);
			continue;
		}

		// status: error
		if (status.compare("ACTIVE") == 0 && f.find(".") == std::string::npos) {
			results.push_back(f);
			continue;
		}

	}

	return results;
}
