#include "imaging.h"

tissuestack::imaging::TissueStackColorMapStore::TissueStackColorMapStore()
{
	if (!tissuestack::utils::System::directoryExists(COLORMAP_PATH))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Color Map Directory does NOT exist!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(COLORMAP_PATH);
	for (std::string f : fileList)
		this->addOrReplaceColorMap(tissuestack::imaging::TissueStackColorMap::fromFile(f.c_str()));

	// add to that the discrete color maps of the label lookups
	const std::unordered_map<std::string, const tissuestack::imaging::TissueStackLabelLookup *> lookups =
			tissuestack::imaging::TissueStackLabelLookupStore::instance()->getAllLabelLookups();

	// walk through entries and copy the colormap
	for (auto lookup = lookups.begin(); lookup != lookups.end(); ++lookup)
		this->addOrReplaceColorMap(lookup->second);
}

void tissuestack::imaging::TissueStackColorMapStore::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry = this->_color_maps.begin(); entry != this->_color_maps.end(); ++entry)
		delete entry->second;

	delete tissuestack::imaging::TissueStackColorMapStore::_instance;
	tissuestack::imaging::TissueStackColorMapStore::_instance = nullptr;
}

 tissuestack::imaging::TissueStackColorMapStore * tissuestack::imaging::TissueStackColorMapStore::instance()
 {
	if (tissuestack::imaging::TissueStackColorMapStore::_instance == nullptr)
		tissuestack::imaging::TissueStackColorMapStore::_instance = new tissuestack::imaging::TissueStackColorMapStore();

	return tissuestack::imaging::TissueStackColorMapStore::_instance;
 }

 const tissuestack::imaging::TissueStackColorMap * tissuestack::imaging::TissueStackColorMapStore::findColorMap(const std::string & id) const
 {
		try
		{
			return this->_color_maps.at(id);

		} catch (std::out_of_range & not_found) {
			return nullptr;
		}
 }

 void tissuestack::imaging::TissueStackColorMapStore::addOrReplaceColorMap(const tissuestack::imaging::TissueStackColorMap * colorMap)
 {
	 if (colorMap == nullptr) return;

	 this->_color_maps[colorMap->getColorMapId()] = colorMap;
 }

 void tissuestack::imaging::TissueStackColorMapStore::addOrReplaceColorMap(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	 if (labelLookup == nullptr) return;

	 this->_color_maps[labelLookup->getLabelLookupId()] =
			 tissuestack::imaging::TissueStackColorMap::fromLabelLookup(labelLookup);
}

tissuestack::imaging::TissueStackColorMapStore * tissuestack::imaging::TissueStackColorMapStore::_instance = nullptr;
