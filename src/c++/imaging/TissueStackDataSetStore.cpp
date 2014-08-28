#include "imaging.h"

tissuestack::imaging::TissueStackDataSetStore::TissueStackDataSetStore()
{
	if (!tissuestack::utils::System::directoryExists(DATASET_PATH))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Data Store Directory does NOT exist!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(DATASET_PATH);
	for (std::string f : fileList)
	try
	{
		tissuestack::logging::TissueStackLogger::instance()->info("Trying to import data set: %s...\n", f.c_str());
		this->addDataSet(tissuestack::imaging::TissueStackDataSet::fromFile(f.c_str()));
		tissuestack::logging::TissueStackLogger::instance()->info("Import successful.\n");
	} catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Could not import data set file '%s' for the following reason:\n%s\n", f.c_str(), bad.what());
	}
}

void tissuestack::imaging::TissueStackDataSetStore::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry : this->_data_sets)
		if (entry.second) delete entry.second;

	delete tissuestack::imaging::TissueStackDataSetStore::_instance;
	tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
}

void tissuestack::imaging::TissueStackDataSetStore::integrateDataBaseResultsIntoDataSetStore(
		std::vector<const tissuestack::imaging::TissueStackImageData *> & dataSets)
{
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
				rec->getResolutionInMm(),
				rec->getLookup());
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
}

tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::instance()
{
	if (tissuestack::imaging::TissueStackDataSetStore::_instance == nullptr)
	{
		tissuestack::imaging::TissueStackDataSetStore::_instance = new tissuestack::imaging::TissueStackDataSetStore();
		// try to pull in additional database information
		try
		{
			std::vector<const tissuestack::imaging::TissueStackImageData *> dataSets =
					tissuestack::database::DataSetDataProvider::queryAll(true);
			if (!dataSets.empty())
				tissuestack::imaging::TissueStackDataSetStore::integrateDataBaseResultsIntoDataSetStore(dataSets);
		}
		catch(std::exception & bad)
		{
			// we'll make a note but not bother
			tissuestack::logging::TissueStackLogger::instance()->error(
					"Could not pull in database info for data sets: %s\n", bad.what());
		}
	}

	return tissuestack::imaging::TissueStackDataSetStore::_instance;
}

const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSetStore::findDataSet(const std::string & id) const
{
	try
	{
		return this->_data_sets.at(id);

	} catch (std::out_of_range & not_found) {
		return nullptr;
	}
}

const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSetStore::findDataSetByDataBaseId(
		const unsigned long long int id) const
{
	if (id == 0)
		return nullptr;

	for (auto dataSet : this->_data_sets)
		if (dataSet.second->getImageData()->getDataBaseId() == id)
			return dataSet.second;

	return nullptr;
}

void tissuestack::imaging::TissueStackDataSetStore::addDataSet(const tissuestack::imaging::TissueStackDataSet * dataSet)
{
	if (dataSet == nullptr) return;

	if (this->findDataSet(dataSet->getDataSetId()) != nullptr) return; // we do not replace in here

	this->_data_sets[dataSet->getDataSetId()] = dataSet;
}

void tissuestack::imaging::TissueStackDataSetStore::replaceDataSet(const tissuestack::imaging::TissueStackDataSet * dataSet)
{
	if (dataSet == nullptr) return;

	const tissuestack::imaging::TissueStackDataSet * existing =
		this->findDataSet(dataSet->getDataSetId());

	if (existing)
		delete existing;

	this->_data_sets[dataSet->getDataSetId()] = dataSet;
}

void tissuestack::imaging::TissueStackDataSetStore::dumpDataSetStoreIntoDebugLog() const
{
	for (auto entry : this->_data_sets)
		entry.second->dumpDataSetContentIntoDebugLog();

}

tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
