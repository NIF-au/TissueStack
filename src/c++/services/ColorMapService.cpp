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
#include "services.h"

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
		const tissuestack::common::ProcessingStrategy * processing_strategy,
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
		sJson = tissuestack::common::NO_RESULTS_JSON;

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", sJson);
	write(file_descriptor, response.c_str(), response.length());
}
