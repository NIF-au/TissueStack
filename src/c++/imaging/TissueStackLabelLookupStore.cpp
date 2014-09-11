#include "networking.h"
#include "imaging.h"

tissuestack::imaging::TissueStackLabelLookupStore::TissueStackLabelLookupStore()
{
	if (!tissuestack::utils::System::directoryExists(LABEL_LOOKUP_PATH) &&
		!tissuestack::utils::System::createDirectory(LABEL_LOOKUP_PATH, 0755))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create label lookup directory!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(LABEL_LOOKUP_PATH);
	for (std::string f : fileList)
	{
		try
		{
			this->addOrReplaceLabelLookup(tissuestack::imaging::TissueStackLabelLookup::fromFile(f.c_str()));
		} catch (std::exception & bad)
		{
			tissuestack::logging::TissueStackLogger::instance()->error(
				"Could not load lookup file '%s' for the following reason:\n%s\n", f.c_str(), bad.what());
		}
	}
}

const bool tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist()
{
	return (tissuestack::imaging::TissueStackLabelLookupStore::_instance != nullptr);
}

void tissuestack::imaging::TissueStackLabelLookupStore::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry : this->_label_lookups)
		if (entry.second) delete entry.second;
	this->_label_lookups.clear();

	delete tissuestack::imaging::TissueStackLabelLookupStore::_instance;
	tissuestack::imaging::TissueStackLabelLookupStore::_instance = nullptr;
}

 tissuestack::imaging::TissueStackLabelLookupStore * tissuestack::imaging::TissueStackLabelLookupStore::instance()
 {
	if (tissuestack::imaging::TissueStackLabelLookupStore::_instance == nullptr)
		tissuestack::imaging::TissueStackLabelLookupStore::_instance = new tissuestack::imaging::TissueStackLabelLookupStore();

	return tissuestack::imaging::TissueStackLabelLookupStore::_instance;
 }

 const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackLabelLookupStore::findLabelLookup(const std::string & id) const
 {
		try
		{
			return this->_label_lookups.at(id);

		} catch (std::out_of_range & not_found) {
			return nullptr;
		}
 }

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackLabelLookupStore::findLabelLookupByFullPath(
		const std::string & id) const
{
	for (auto l : this->_label_lookups)
		if (l.second->getLabelLookupId(true).compare(id) == 0)
			return l.second;

	return nullptr;
}

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackLabelLookupStore::findLabelLookupByDataBaseId(
		const unsigned long long int id) const
{
	if (id == 0) return nullptr;

	for (auto l : this->_label_lookups)
		if (l.second->getDataBaseId() == id)
			return l.second;

	return nullptr;
}


void tissuestack::imaging::TissueStackLabelLookupStore::addOrReplaceLabelLookup(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	 if (labelLookup == nullptr) return;

	 this->_label_lookups[labelLookup->getLabelLookupId()] = labelLookup;
}

const std::unordered_map<std::string, const tissuestack::imaging::TissueStackLabelLookup *> tissuestack::imaging::TissueStackLabelLookupStore::getAllLabelLookups() const
 {
	 return this->_label_lookups;
 }

 void tissuestack::imaging::TissueStackLabelLookupStore::dumpAllLabelLookupsToDebugLog() const
 {
	for (auto entry = this->_label_lookups.begin(); entry != this->_label_lookups.end(); ++entry)
		entry->second->dumpLabelLookupToDebugLog();
 }

tissuestack::imaging::TissueStackLabelLookupStore * tissuestack::imaging::TissueStackLabelLookupStore::_instance = nullptr;
