#include "imaging.h"

tissuestack::imaging::TissueStackLabelLookupStore::TissueStackLabelLookupStore()
{
	if (!tissuestack::utils::System::directoryExists(LABEL_LOOKUP_PATH))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Label Lookup Directory does NOT exist!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(LABEL_LOOKUP_PATH);
	for (std::string f : fileList)
	try
	{
		this->addOrReplaceLabelLookup(tissuestack::imaging::TissueStackLabelLookup::fromFile(f.c_str()));
	} catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Could not load lookup file '%s' for the following reason:\n%s\n", f.c_str(), bad.what());
	}
}

void tissuestack::imaging::TissueStackLabelLookupStore::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry = this->_label_lookups.begin(); entry != this->_label_lookups.end(); ++entry)
		delete entry->second;

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
