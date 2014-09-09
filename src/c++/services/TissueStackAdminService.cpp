#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

const std::string tissuestack::services::TissueStackAdminService::SUB_SERVICE_ID = "ADMIN";

tissuestack::services::TissueStackAdminService::TissueStackAdminService() {
	/* do not need session */
	this->addMandatoryParametersForRequest("UPLOAD_DIRECTORY", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("DATA_SET_RAW_FILES", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("UPLOAD_PROGRESS",
		std::vector<std::string>{ "FILE"});
	this->addMandatoryParametersForRequest("PROGRESS",
		std::vector<std::string>{ "TASK_ID"});
	/* need session */
	this->addMandatoryParametersForRequest("UPLOAD",
		std::vector<std::string>{ "SESSION"});
	this->addMandatoryParametersForRequest("ADD_DATASET",
		std::vector<std::string>{ "SESSION" , "FILENAME"});
	this->addMandatoryParametersForRequest("TOGGLE_TILING",
		std::vector<std::string>{ "SESSION" , "ID", "FLAG"});
	this->addMandatoryParametersForRequest("CANCEL",
		std::vector<std::string>{ "SESSION" , "TASK_ID"});
};

tissuestack::services::TissueStackAdminService::~TissueStackAdminService() {};

void tissuestack::services::TissueStackAdminService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::TissueStackAdminService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::string json = tissuestack::common::NO_RESULTS_JSON;

	// do the requests that need no session first
	if (action.compare("UPLOAD_DIRECTORY") == 0)
		json = this->handleUploadDirectoryRequest(request);
	else if (action.compare("DATA_SET_RAW_FILES") == 0)
		json = this->handleDataSetRawFilesRequest(request);
	else if (action.compare("UPLOAD_PROGRESS") == 0)
		json = this->handleUploadProgressRequest(request);
	else if (action.compare("PROGRESS") == 0)
		json = this->handleProgressRequest(request);
	else
	{
		// the following resources need a valid session
		if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
			request->getRequestParameter("SESSION")))
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Invalid Session! Please Log In.");
		}

		if (action.compare("TOGGLE_TILING") == 0)
			json = this->handleSetTilingRequest(request);
		else if (action.compare("CANCEL") == 0)
			json = this->handleTaskCancellationRequest(request);
		else if (action.compare("UPLOAD") == 0)
			json = this->handleUploadRequest(request);
		else if (action.compare("ADD_DATASET") == 0)
			json = this->handleDataSetAdditionRequest(request);
	}

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json);
	write(file_descriptor, response.c_str(), response.length());
}

const std::string tissuestack::services::TissueStackAdminService::handleSetTilingRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const unsigned long long int id =
		strtoull(request->getRequestParameter("ID", true).c_str(), NULL, 10);

	const std::string flag =
		request->getRequestParameter("FLAG", true);

	const bool response =
		tissuestack::database::DataSetDataProvider::setIsTiledFlag(
			id, (flag.compare("TRUE") == 0) ? true : false);

	if (response)
		return "{\"response\":\"Toggled pre-tiling/image flag successfully\"}";
	else
		return tissuestack::common::NO_RESULTS_JSON;
}

const std::string tissuestack::services::TissueStackAdminService::handleTaskCancellationRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string task_id =
		request->getRequestParameter("TASK_ID", true);

	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(task_id);
	if (hit == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");

	const std::string fileName =
		hit->getInputImageData()->getFileName();
	const std::string progress = std::to_string(hit->getProgress());
	// cancel
	tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(task_id, true);

	return std::string("{\"response\": {\"filename\": \"") +
		fileName + "\", \"progress\":" +
		progress + "}}";
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	//TODO: implement
	return "NOT IMPLEMENTED";
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetAdditionRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string fileName = request->getRequestParameter("FILENAME");
	const std::string description = request->getRequestParameter("DESCRIPTION");

	// check for existing file in data directory
	if (!tissuestack::utils::System::fileExists(fileName))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Data Set file does not exist!");

	const unsigned int pos = fileName.find_last_of("/");
	std::string finalDestination = DATASET_PATH;
	if (pos != std::string::npos)
		finalDestination += ("/" + fileName.substr(pos+1));
	else
		finalDestination += ("/" + fileName);

	if (tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(finalDestination))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Data Set referring to same file exists already!");

	// move raw file !
	if (rename(fileName.c_str(), finalDestination.c_str()) != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not move data set!");

	std::unique_ptr<const tissuestack::imaging::TissueStackImageData> dataSet;
	unsigned long long int id = 0;
	try
	{
		dataSet.reset(tissuestack::imaging::TissueStackImageData::fromFile(finalDestination));
		if (!dataSet->isRaw())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Only .raw files can be added!");

		if (tissuestack::database::DataSetDataProvider::addDataSet(
				dataSet.get(), description) != (dataSet->getNumberOfDimensions() + 1))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to persist all dimensions of the data set!");

		// now that we are persisted => add us to the memory data store
		id = dataSet->getDataBaseId();
		tissuestack::imaging::TissueStackDataSetStore::instance()->addDataSet(
			tissuestack::imaging::TissueStackDataSet::fromTissueStackImageData(dataSet.release()));

		/* this is not needed, strictly speaking s unless we have a reason, we'll omit it!
		// now go into the db to double check and retrieve default values
		const tissuestack::imaging::TissueStackImageData * correspondingRecord =
				tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(dataSet->getDataBaseId(), true);

		// integrate the record
		std::vector<const tissuestack::imaging::TissueStackImageData *> tmp;
		tmp.push_back(correspondingRecord);
		tissuestack::imaging::TissueStackDataSetStore::instance()->integrateDataBaseResultsIntoDataSetStore(tmp);
		*/
	} catch (const std::exception & e)
	{
		// rollback
		try
		{
			if (id != 0)
			{
				if (tissuestack::database::DataSetDataProvider::eraseDataSet(id))
					tissuestack::imaging::TissueStackDataSetStore::instance()->removeDataSetByDataBaseId(id);
			}
		} catch(...)
		{
			// ignored
		}
		// undo moving of file
		rename(finalDestination.c_str(), fileName.c_str());

		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to persist data set: %s", e.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failed to persist data set!");
	}

	std::string json = "{\"response\": ";
	json +=
		tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(finalDestination)->getImageData()->toJson(true)
		+ "}";

	return json;
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadDirectoryRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string sDisplayRawOnly =
			request->getRequestParameter("DISPLAY_RAW_ONLY", true);
	const bool bDisplayRawOnly =
		(!sDisplayRawOnly.empty() && sDisplayRawOnly.compare("TRUE") == 0) ?
			true : false;
	const std::string sDisplayConversionFormatsOnly =
			request->getRequestParameter("DISPLAY_CONVERSION_FORMATS_ONLY", true);
	const bool bDisplayConversionFormatsOnly =
		(!sDisplayConversionFormatsOnly.empty() && sDisplayConversionFormatsOnly.compare("TRUE") == 0) ?
			true : false;

	std::vector<std::string> results;

	const std::vector<std::string> filesInUploadDirectory =
		tissuestack::utils::System::getFilesInDirectory(UPLOAD_PATH);

	for (auto f : filesInUploadDirectory)
	{
		if (tissuestack::services::TissueStackTaskQueue::instance()->doesTaskExistForDataSet(f))
			continue;

		if (bDisplayRawOnly)
		{
			if (f.length() < 4)
				continue;

			if (!(f.substr(f.length()-4).compare(".raw") == 0
					|| f.substr(f.length()-4).compare(".RAW") == 0))
				continue;
		}

		if (bDisplayConversionFormatsOnly)
		{
			if (f.length() < 4)
				continue;
			std::string ext = f.substr(f.length()-4);
			std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

			if (!(ext.compare(".MNC") == 0
					|| ext.compare(".NII") == 0
					|| ext.compare("I.GZ") == 0))
				continue;
		}

		const unsigned int pos = f.find_last_of("/");
		if (pos != std::string::npos)
			results.push_back(f.substr(pos+1));
		else
			results.push_back(f);
	}

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto r : results)
	{
		if (i !=0)
			json << ",";
		json << "\"" << r << "\"";
		i++;
	}

	json << "]}";

	return json.str();
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetRawFilesRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	std::vector<std::string> results;

	const std::vector<std::string> dataSets =
		tissuestack::imaging::TissueStackDataSetStore::instance()->getRawFileDataSetFiles();

	for (auto ds : dataSets)
	{
		if (tissuestack::services::TissueStackTaskQueue::instance()->doesTaskExistForDataSet(ds))
			continue;

		const unsigned int pos = ds.find_last_of("/");
		if (pos != std::string::npos)
			results.push_back(ds.substr(pos+1));
		else
			results.push_back(ds);
	}

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto r : results)
	{
		if (i !=0)
			json << ",";
		json << "\"" << r << "\"";
		i++;
	}

	json << "]}";

	return json.str();
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadProgressRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	//TODO: implement
	return "NOT IMPLEMENTED";
}

const std::string tissuestack::services::TissueStackAdminService::handleProgressRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string task_id =
		request->getRequestParameter("TASK_ID", true);

	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(task_id);
	if (hit == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");

	return std::string("{\"response\": {\"filename\": \"") +
		hit->getInputImageData()->getFileName() + "\", \"progress\":" +
		std::to_string(hit->getProgress()) + "}}";

}
