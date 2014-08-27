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
				tissuestack::services::TissueStackServiceError::NO_RESULTS);
		write(file_descriptor, response.c_str(), response.length());
		return;
	}

	// if we have stuff in memory, take it from there, otherwise go back fishing in database
	for (unsigned int i=0;i<dataSets.size();i++)
	{
		const tissuestack::imaging::TissueStackImageData * rec = dataSets[i];
		const tissuestack::imaging::TissueStackDataSet * foundDataSet =
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(rec->getFileName());
		if (foundDataSet)
		{
			const_cast<tissuestack::imaging::TissueStackImageData *>(foundDataSet->getImageData())->setMembersFromDataBaseInformation(
				rec->getDataBaseId(),
				rec->getDescription(),
				rec->isTiled(),
				rec->getZoomLevels(),
				rec->getOneToOneZoomLevel(),
				rec->getResolutionInMm());
			delete rec;
			dataSets[i] = foundDataSet->getImageData();
			continue;
		}

		// we have a data set that solely exists in the data base, i.e. has no corresponding physical raw file
		// let's query all of its info and then add it to the data set store
		foundDataSet = // check whether we have already queried it once and added it to the memory store
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSetByDataBaseId(rec->getDataBaseId());
		if (foundDataSet == nullptr) // we did not: fetch the records
			foundDataSet =
				tissuestack::imaging::TissueStackDataSet::fromDataBaseRecordWithId(rec->getDataBaseId(), true);
		if (foundDataSet == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Data Set Record Missing!");
		tissuestack::imaging::TissueStackDataSetStore::instance()->addDataSet(foundDataSet);

		delete rec;
		dataSets[i] = foundDataSet->getImageData();
	}

	// iterate over results and marshall them
	std::ostringstream json;
	json << "{ \"response\": [ ";
	int i=0;
	for (const tissuestack::imaging::TissueStackImageData * dataSet : dataSets)
	{
		if (i != 0) json << ",";
		json << dataSet->toJson(bIncludePlanes).c_str();
		i++;
	}
	json << "] }";

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
}
