#include "networking.h"
#include "imaging.h"
#include "database.h"
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
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);
	const std::string includePlanes =
			request->getRequestParameter("include_planes", true);
	const bool bIncludePlanes =
			(!includePlanes.empty() && includePlanes.compare("TRUE") == 0) ? true : false;

	std::vector<const tissuestack::imaging::TissueStackImageData *> dataSets;
	if (action.compare("ALL") == 0)
	{
		const std::string sOffset = request->getRequestParameter("OFFSET");
		const std::string sMaxRecords = request->getRequestParameter("MAX_RECORDS");

		const unsigned int offset =
			sOffset.empty() ? 0 :
				static_cast<unsigned int>(strtoull(sOffset.c_str(), NULL, 10));
		const unsigned int max_records = sMaxRecords.empty() ? tissuestack::database::DataSetDataProvider::MAX_RECORDS :
				static_cast<unsigned int>(strtoull(sMaxRecords.c_str(), NULL, 10));

		dataSets =
			tissuestack::database::DataSetDataProvider::queryAll(bIncludePlanes, offset, max_records);
	}
	else if (action.compare("QUERY") == 0)
	{
		dataSets =
			tissuestack::database::DataSetDataProvider::queryById(
				strtoull(request->getRequestParameter("ID").c_str(), NULL, 10));
	}

	// we return if no results
	if (dataSets.empty())
	{
		const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json",
				tissuestack::common::NO_RESULTS_JSON);
		write(file_descriptor, response.c_str(), response.length());
		return;
	}

	tissuestack::imaging::TissueStackDataSetStore::integrateDataBaseResultsIntoDataSetStore(dataSets);

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
}
