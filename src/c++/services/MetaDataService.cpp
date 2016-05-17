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

const std::string tissuestack::services::MetaDataService::SUB_SERVICE_ID = "METADATA";



tissuestack::services::MetaDataService::MetaDataService() {
	this->addMandatoryParametersForRequest("DATASET_LIST",
		std::vector<std::string>{ "SESSION" });
};

tissuestack::services::MetaDataService::~MetaDataService() {};

void tissuestack::services::MetaDataService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::MetaDataService::streamResponse(
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


	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json);
	write(file_descriptor, response.c_str(), response.length());
}

const std::string tissuestack::services::MetaDataService::handleDataSetListRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const {

	const std::vector<const tissuestack::database::DataSetInfo *> results =
		tissuestack::database::ServicesDataProvider::queryAllDataSets();

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto ds : results)
	{
		if (i !=0)
				json << ",";

		json << "{";
		json << "\"id\": " << std::to_string(ds->getId()) << ",";
		json << "\"filename\": " << ds->getFileName();
		if (!ds->getDescription().empty())
			json << ",\"description\": " << ds->getDescription();
		json << "}";
		i++;
		delete ds;
	}
	json << "]}";

	return json.str();
}

