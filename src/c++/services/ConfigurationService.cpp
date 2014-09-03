#include "services.h"

const std::string tissuestack::services::ConfigurationService::SUB_SERVICE_ID = "CONFIGURATION";



tissuestack::services::ConfigurationService::ConfigurationService() {
	this->addMandatoryParametersForRequest("ALL", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("VERSION", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("SUPPORTS_TILE_SERVICE", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("SUPPORTS_IMAGE_SERVICE", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("QUERY", std::vector<std::string>{"KEY"});
};

tissuestack::services::ConfigurationService::~ConfigurationService() {};

void tissuestack::services::ConfigurationService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::ConfigurationService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::vector<const tissuestack::database::Configuration *> conf;
	if (action.compare("ALL") == 0)
		conf =
			tissuestack::database::ConfigurationDataProvider::queryAllConfigurations();
	else if (action.compare("VERSION") == 0)
		conf.push_back(
				tissuestack::database::ConfigurationDataProvider::queryConfigurationById("version"));
	else if (action.compare("SUPPORTS_TILE_SERVICE") == 0)
		conf.push_back(tissuestack::database::ConfigurationDataProvider::queryConfigurationById("tiles"));
	else if (action.compare("SUPPORTS_IMAGE_SERVICE") == 0)
		conf.push_back(tissuestack::database::ConfigurationDataProvider::queryConfigurationById("images"));
	else if (action.compare("QUERY") == 0)
		conf.push_back(
				tissuestack::database::ConfigurationDataProvider::queryConfigurationById(
						request->getRequestParameter("KEY")));

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
		json << tissuestack::common::NO_RESULTS_JSON;

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
}
