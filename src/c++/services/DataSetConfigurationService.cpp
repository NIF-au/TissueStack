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
			request->getRequestParameter("planes", true);
	const bool bIncludePlanes =
			(!includePlanes.empty() && includePlanes.compare("TRUE") == 0) ? true : false;

	std::vector<const tissuestack::imaging::TissueStackImageData *> dataSets;
	if (action.compare("ALL") == 0)
		dataSets =
			tissuestack::database::DataSetDataProvider::queryAll();
	else if (action.compare("QUERY") == 0)
		dataSets.push_back(
				tissuestack::database::DataSetDataProvider::queryById(
						strtoull(request->getRequestParameter("ID").c_str(), NULL, 10)));

	// if we have stuff in memory, take it from there, otherwise go back fishing in database
	for (const tissuestack::imaging::TissueStackImageData * rec : dataSets)
	{
		const tissuestack::imaging::TissueStackDataSet * foundDataSet =
				tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(rec->getFileName());
		if (foundDataSet)
		{
			if (bIncludePlanes) // we'll use the image data from the global store and copy over the bits from the data base
			{
				const_cast<tissuestack::imaging::TissueStackImageData *>(foundDataSet->getImageData())->setMembersFromDataBaseInformation(
					rec->isTiled(),
					rec->getZoomLevels(),
					rec->getOneToOneZoomLevel(),
					rec->getResolutionInMm());
				// swap pointers and delete the pointer to the db search results
				const tissuestack::imaging::TissueStackImageData * tmp = rec;
				rec = foundDataSet->getImageData();
				delete tmp;
			}
			continue;
		}

		// we have a data set that solely exists in the data base, i.e. has no corresponding physical raw file
		// let's query all of its info and then add it to the data set store
		foundDataSet =
				tissuestack::imaging::TissueStackDataSet::fromDataBaseRecordWithId(rec->getDataBaseId(), bIncludePlanes);
		if (foundDataSet == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Data Set Record Missing!");
		tissuestack::imaging::TissueStackDataSetStore::instance()->addDataSet(foundDataSet);

		// swap pointers and delete the previous query pointers
		const tissuestack::imaging::TissueStackImageData * tmp = rec;
		rec = foundDataSet->getImageData();
		delete tmp;
	}

	// iterate over results and marshall them
	std::ostringstream json;
	if (!dataSets.empty() && dataSets[0] != nullptr)
	{
		json << "{ \"response\": [ ";
		int i=0;
		for (const tissuestack::imaging::TissueStackImageData * dataSet : dataSets)
		{
			if (i != 0) json << ",";
			json << dataSet->toJson().c_str();
			i++;
		}
		json << "] }";
	} else
		json << tissuestack::services::TissueStackServiceError::NO_RESULTS;

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
}
