#include "services.h"
#include "imaging.h"

const std::string tissuestack::services::ColorMapService::SUB_SERVICE_ID = "COLORMAPS";



tissuestack::services::ColorMapService::ColorMapService() {
	this->addMandatoryParametersForRequest("ALL", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("QUERY", std::vector<std::string>{"NAME"});
};

tissuestack::services::ColorMapService::~ColorMapService() {};

void tissuestack::services::ColorMapService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::ColorMapService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);
	const bool originalColorMappingFileContents =
			request->getRequestParameter("FINAL").empty() ? true : false;

	std::ostringstream json;

	if (action.compare("ALL") == 0)
		json << tissuestack::imaging::TissueStackColorMapStore::instance()->toJson(originalColorMappingFileContents);
	else if (action.compare("QUERY") == 0)
	{
		const tissuestack::imaging::TissueStackColorMap * map =
				tissuestack::imaging::TissueStackColorMapStore::instance()->findColorMap(
					request->getRequestParameter("NAME"));
		if (map == nullptr)
			json << "";
		else
			json << "{" << map->toJson(originalColorMappingFileContents) << "}";
	}
	std::string sJson = json.str();
	if (sJson.empty())
		sJson = tissuestack::services::TissueStackServiceError::NO_RESULTS;

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", sJson);
	write(file_descriptor, response.c_str(), response.length());
}
