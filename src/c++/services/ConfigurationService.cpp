#include "services.h"

const std::string tissuestack::services::ConfigurationService::SUB_SERVICE_ID = "CONFIGURATION";

tissuestack::services::ConfigurationService::ConfigurationService() {};
tissuestack::services::ConfigurationService::~ConfigurationService() {};

void tissuestack::services::ConfigurationService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	// TODO: implement
}

void tissuestack::services::ConfigurationService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	// TODO: implement
	std::ostringstream json;
	json << "{ \"response\": [ ";
	const std::vector<tissuestack::database::Configuration *> conf =
			tissuestack::database::ConfigurationDataProvider::queryAllConfigurations();
	int i=0;
	for (tissuestack::database::Configuration * c : conf)
	{
		if (i != 0) json << ",";
		json << c->getJson().c_str();
		delete c;
		i++;
	}
	json << "] }";

	//const std::string json = tissuestack::database::ConfigurationDataProvider::queryConfigurationById("color_maps")->getJson();
	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
}
