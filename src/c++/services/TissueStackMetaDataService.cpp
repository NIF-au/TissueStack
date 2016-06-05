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
