#include "networking.h"
#include "imaging.h"

tissuestack::imaging::TissueStackColorMapStore::TissueStackColorMapStore()
{
	if (!tissuestack::utils::System::directoryExists(COLORMAP_PATH) &&
		!tissuestack::utils::System::createDirectory(COLORMAP_PATH, 0755))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create color map directory!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(COLORMAP_PATH);
	for (std::string f : fileList)
	{
		try
		{
			this->addOrReplaceColorMap(tissuestack::imaging::TissueStackColorMap::fromFile(f.c_str()));
		} catch (std::exception & bad)
		{
			if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
				tissuestack::logging::TissueStackLogger::instance()->error(
					"Could not load color map file '%s' for the following reason:\n%s\n", f.c_str(), bad.what());
		}
	}

	if (!tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist())
		return;

	// add to that the discrete color maps of the label lookups
	const std::unordered_map<std::string, const tissuestack::imaging::TissueStackLabelLookup *> lookups =
			tissuestack::imaging::TissueStackLabelLookupStore::instance()->getAllLabelLookups();

	// walk through entries and copy the colormap
	for (auto lookup : lookups)
	{
		try
		{
			this->addOrReplaceColorMap(lookup.second);
		} catch (std::exception & bad)
		{
			if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
				tissuestack::logging::TissueStackLogger::instance()->error(
					"Could not load color map from lookup file '%s' for the following reason:\n%s\n",
					lookup.second->getLabelLookupId().c_str(), bad.what());
		}
	}
}

void tissuestack::imaging::TissueStackColorMapStore::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry = this->_color_maps.begin(); entry != this->_color_maps.end(); ++entry)
		if (entry->second) delete entry->second;

	delete tissuestack::imaging::TissueStackColorMapStore::_instance;
	tissuestack::imaging::TissueStackColorMapStore::_instance = nullptr;
}

const bool tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist()
{
	return (tissuestack::imaging::TissueStackColorMapStore::_instance != nullptr);
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

	 if (this->findColorMap(colorMap->getColorMapId()))
		 delete this->_color_maps[colorMap->getColorMapId()];

	 this->_color_maps[colorMap->getColorMapId()] = colorMap;
 }

 void tissuestack::imaging::TissueStackColorMapStore::addOrReplaceColorMap(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	 if (labelLookup == nullptr) return;

	 if (this->findColorMap(labelLookup->getLabelLookupId()))
		 delete this->_color_maps[labelLookup->getLabelLookupId()];

	 this->_color_maps[labelLookup->getLabelLookupId()] =
			 tissuestack::imaging::TissueStackColorMap::fromLabelLookup(labelLookup);
}

const std::string tissuestack::imaging::TissueStackColorMapStore::toJson(bool originalColorMapContents) const
{
	if (this->_color_maps.empty())
		return "";

	std::ostringstream json;
	json << "{";
	unsigned short i=0;

	for (auto entry = this->_color_maps.begin(); entry != this->_color_maps.end(); ++entry)
	{
		if (i !=0)
			json << ",";

		json << entry->second->toJson(originalColorMapContents);
		i++;
	}
	json << "}";

	return json.str();
}

void tissuestack::imaging::TissueStackColorMapStore::dumpAllColorMapsToDebugLog() const
{
	for (auto entry = this->_color_maps.begin(); entry != this->_color_maps.end(); ++entry)
		entry->second->dumpColorMapToDebugLog();
}

tissuestack::imaging::TissueStackColorMapStore * tissuestack::imaging::TissueStackColorMapStore::_instance = nullptr;
