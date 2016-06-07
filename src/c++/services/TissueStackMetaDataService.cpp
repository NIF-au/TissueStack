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

	const std::string task_id =
		request->getRequestParameter("TASK_ID", true);

	// TODO: loop over files in directory and filter if necessary
	return "";
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

	// check in memory first
	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(task_id);
	if (hit) {
		// good we don't need to fish in the file
		if ((task_type.compare("TILING") == 0 &&
				hit->getType() == tissuestack::services::TissueStackTaskType::CONVERSION) ||
			(task_type.compare("CONVERSION") == 0 &&
				hit->getType() == tissuestack::services::TissueStackTaskType::TILING))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Given task type does not correspond to actual, e.g. you gave a supposed tile task id for a conversion or vice versa!");

		std::ostringstream json;
		json << "{\"response\": ";
		json << "{\"filename\": \"";
		json << hit->getInputFileName();
		json << "\", \"progress\": ";
		json << hit->getProgress();
		json << ",\"status\": \"";
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
				json << "canceled";
				break;
			default:
				json << "unknown";
		}
		json << "\"}}";
		return json.str();
	} else {
		// we have to go and look for the file....
		// TODO: check a few options: done error
		//tissuestack::services::TissueStackTaskQueue::getTasksDirectory() + "/" + task_id

	}

	THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");
}

inline std::string tissuestack::services::TissueStackMetaDataService::getStatusFromTaskFile(
		const std::string & task_id,
		const std::string & task_type)
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
	THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
		"Given task type does not correspond to actual, e.g. you gave a supposed tile task id for a conversion or vice versa!");


	// get the progress/total figures
	if (sliceProgress > totalSlices) // we cannot have more progress than total work...
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Erroneous Progress values!");

	float progress = 0;
	if (totalSlices != 0) {
		progress = (static_cast<float>(sliceProgress) / static_cast<float>(totalSlices)) *
			static_cast<float>(100);
	}

	std::ostringstream json;
	json << "{\"response\": ";
	json << "{\"filename\": \"";
	json << in_file;
	json << "\", \"progress\": ";
	json << std::to_string(progress);
	json << ",\"status\": \"";
	// TODO: gather status from file name and progress
	json << "\"}}";
	return json.str();
}
