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
	for (auto entry = this->_data_sets.begin(); entry != this->_data_sets.end(); ++entry)
		delete entry->second;

	delete tissuestack::imaging::TissueStackDataSetStore::_instance;
	tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
}

tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::instance()
{
if (tissuestack::imaging::TissueStackDataSetStore::_instance == nullptr)
	tissuestack::imaging::TissueStackDataSetStore::_instance = new tissuestack::imaging::TissueStackDataSetStore();

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

void tissuestack::imaging::TissueStackDataSetStore::addDataSet(const tissuestack::imaging::TissueStackDataSet * dataSet)
{
	if (dataSet == nullptr) return;

	if (this->findDataSet(dataSet->getDataSetId()) != nullptr) return; // we do not replace

	this->_data_sets[dataSet->getDataSetId()] = dataSet;
}

void tissuestack::imaging::TissueStackDataSetStore::dumpDataSetStoreIntoDebugLog() const
{
	for (auto entry : this->_data_sets)
		entry.second->dumpDataSetContentIntoDebugLog();

}

tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
