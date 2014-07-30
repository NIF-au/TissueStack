#include "imaging.h"

tissuestack::imaging::TissueStackDataSetStore::TissueStackDataSetStore()
{
	// TODO: read data directory and add data sets
}

void tissuestack::imaging::TissueStackDataSetStore::purgeInstance()
{
	delete tissuestack::imaging::TissueStackDataSetStore::_instance;
	tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
}

 tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::instance()
 {
	if (tissuestack::imaging::TissueStackDataSetStore::_instance == nullptr)
		tissuestack::imaging::TissueStackDataSetStore::_instance = new tissuestack::imaging::TissueStackDataSetStore();

	return tissuestack::imaging::TissueStackDataSetStore::_instance;
 }

 const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSetStore::findDataSet(const std::string id) const
 {
		try
		{
			return this->_data_sets.at(id);

		} catch (std::out_of_range & not_found) {
			return nullptr;
		}
 }

 void tissuestack::imaging::TissueStackDataSetStore::addOrReplaceDataSet(const tissuestack::imaging::TissueStackDataSet * dataSet)
 {
	 if (dataSet == nullptr) return;

	 this->_data_sets[dataSet->getDataSetId()] = dataSet;
 }

tissuestack::imaging::TissueStackDataSetStore * tissuestack::imaging::TissueStackDataSetStore::_instance = nullptr;
