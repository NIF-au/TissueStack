#include "services.h"

const std::string tissuestack::services::DataSetConfigurationService::SUB_SERVICE_ID = "DATA";



tissuestack::services::DataSetConfigurationService::DataSetConfigurationService() {
	this->addMandatoryParametersForRequest("ALL", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("QUERY", std::vector<std::string>{"ID"});
};

tissuestack::services::DataSetConfigurationService::~DataSetConfigurationService() {};

void tissuestack::services::DataSetConfigurationService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::DataSetConfigurationService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::vector<const tissuestack::imaging::TissueStackImageData *> conf;
	if (action.compare("ALL") == 0)
		conf =
			tissuestack::database::DataSetDataProvider::queryAll();
	else if (action.compare("QUERY") == 0)
		conf.push_back(
				tissuestack::database::DataSetDataProvider::queryById(
						strtoull(request->getRequestParameter("ID").c_str(), NULL, 10)));

	// TODO: implement
	// if we have stuff in memory, take it from there, otherwise go back fishing in database


	/*
	 * TODO: marshall
	std::ostringstream json;
	if (!conf.empty() && conf[0] != nullptr)
	{
		json << "{ \"response\": [ ";
		int i=0;
		for (const tissuestack::database::Configuration * c : conf)
		{
			if (i != 0) json << ",";
			json << c->getJson().c_str();
			delete c;
			i++;
		}
		json << "] }";
	} else
		json << tissuestack::services::TissueStackServiceError::NO_RESULTS;
	*/
	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", "NOT IMPLEMENTED YET");
	write(file_descriptor, response.c_str(), response.length());
}
