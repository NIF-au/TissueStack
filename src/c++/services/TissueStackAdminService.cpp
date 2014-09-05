#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

const std::string tissuestack::services::TissueStackAdminService::SUB_SERVICE_ID = "ADMIN";

tissuestack::services::TissueStackAdminService::TissueStackAdminService() {
	this->addMandatoryParametersForRequest("UPLOAD",
		std::vector<std::string>{ "SESSION"});
	this->addMandatoryParametersForRequest("UPLOAD_PROGRESS",
		std::vector<std::string>{ "SESSION" , "FILE"});
	this->addMandatoryParametersForRequest("UPLOAD_DIRECTORY", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("DATA_SET_RAW_FILES", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("ADD_DATASET",
		std::vector<std::string>{ "SESSION" , "FILENAME"});
	this->addMandatoryParametersForRequest("TOGGLE_TILING",
		std::vector<std::string>{ "SESSION" , "ID", "FLAG"});
	this->addMandatoryParametersForRequest("PROGRESS",
		std::vector<std::string>{ "SESSION" , "TASK_ID"});
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

	std::ostringstream json;

	if (action.compare("UPLOAD_DIRECTORY") == 0)
	{
		json << "{ \"response\": \"";
	}
	// TODO: implement

	/*
	// iterate over results and marshall them
	std::ostringstream json;
	json << "{ \"response\": [ ";
	int i=0;
	for (const tissuestack::imaging::TissueStackImageData * dataSet : dataSets)
	{
		if (i != 0) json << ",";
		json << dataSet->toJson(bIncludePlanes, false).c_str();
		i++;
	}
	json << "] }";

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
	*/
}
