/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "networking.h"
#include "imaging.h"
#include "database.h"

tissuestack::imaging::TissueStackColorMapStore::TissueStackColorMapStore()
{
	this->updateColorMapStore(true);
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

	 //if (this->findColorMap(colorMap->getColorMapId()))
	 //	 delete this->_color_maps[colorMap->getColorMapId()];

	 this->_color_maps[colorMap->getColorMapId()] = colorMap;
 }

 void tissuestack::imaging::TissueStackColorMapStore::addOrReplaceColorMap(
		 const tissuestack::imaging::TissueStackLabelLookup * labelLookup,
		 const time_t lastModified)
{
	 if (labelLookup == nullptr) return;

	 tissuestack::imaging::TissueStackColorMap * colorFromLabel =
		const_cast<tissuestack::imaging::TissueStackColorMap *>(
		tissuestack::imaging::TissueStackColorMap::fromLabelLookup(labelLookup));
	 colorFromLabel->setLastModified(lastModified);

	 const tissuestack::imaging::TissueStackColorMap * oldPointer =
		this->findColorMap(labelLookup->getLabelLookupId());
	 if (oldPointer != nullptr)
		 const_cast<tissuestack::imaging::TissueStackColorMap *>(oldPointer)->setUpdateFlag(true);

	 this->_color_maps[labelLookup->getLabelLookupId()] = colorFromLabel;

	 if (oldPointer != nullptr)
		 delete oldPointer;

}

void tissuestack::imaging::TissueStackColorMapStore::updateColorMapStore(bool initial)
{
	const std::string dir = tissuestack::imaging::TissueStackColorMapStore::getColorMapDirectory();
	if (!tissuestack::utils::System::directoryExists(dir) &&
		!tissuestack::utils::System::createDirectory(dir, 0755))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create color map directory!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(dir);
	for (std::string f : fileList)
	{
		try
		{
			if (f.rfind("/.") != std::string::npos) // skip .files
				continue;

			std::string shortPath = f;
			size_t lastSlash = 0;
			lastSlash = f.rfind("/");
			if (lastSlash != std::string::npos)
			{
				lastSlash++;
				shortPath = f.substr(lastSlash, f.length()-lastSlash);
			}

			if (tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist() &&
				tissuestack::imaging::TissueStackLabelLookupStore::instance()->findLabelLookup(shortPath))
			{
				tissuestack::logging::TissueStackLogger::instance()->info(
					"Failed to add color map '%s' as there is a lookup file by the same name!", shortPath.c_str());
				continue;
			}

			tissuestack::imaging::TissueStackColorMap * newColorMap =
				const_cast<tissuestack::imaging::TissueStackColorMap *>(
					initial ? // first load (from directory)
						tissuestack::imaging::TissueStackColorMap::fromFile(f) :
						this->findColorMap(shortPath)); // update of already loaded file

			time_t lastModificationTime =
					newColorMap && newColorMap->getLastModified() > 0 ?
						newColorMap->getLastModified() :
					tissuestack::utils::System::getLastModifiedTime(f);

			if (initial) // we set the last modified for the inital load minus 1 to force addition
				lastModificationTime -= 1;
			else if (!initial && newColorMap == nullptr) // this happens if a new file has been added
			{
				tissuestack::logging::TissueStackLogger::instance()->info(
					"Adding new color map file '%s'", f.c_str());

				newColorMap =
					const_cast<tissuestack::imaging::TissueStackColorMap *>(
					tissuestack::imaging::TissueStackColorMap::fromFile(f));
				lastModificationTime -= 1;
			}

			// do the time comparison and decide to not update if there was no file change
			const time_t latestModification =
				tissuestack::utils::System::hasFileBeenModifiedSince(f, lastModificationTime);

			if (latestModification == 0) // skip
				continue;

			newColorMap->setUpdateFlag(true);
			if (!initial) newColorMap->updateColorMap(f);
			this->addOrReplaceColorMap(newColorMap);
			newColorMap->setUpdateFlag(false);
			newColorMap->setLastModified( // we set the last modified for added files but deduct 1 so that we force addition
					latestModification);
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
			tissuestack::imaging::TissueStackColorMap * newColorMap =
				const_cast<tissuestack::imaging::TissueStackColorMap *>(
					this->findColorMap(lookup.second->getLabelLookupId()));

			time_t lastModificationTime =
					newColorMap && newColorMap->getLastModified() > 0 ?
							newColorMap->getLastModified() :
					lookup.second->getLastModified();

			if (newColorMap == nullptr)
				lastModificationTime -= 1;

			// do the time comparison and decide to not update if there was no file change
			const time_t latestModification =
				tissuestack::utils::System::hasFileBeenModifiedSince(lookup.second->getLabelLookupId(true), lastModificationTime);

			if (latestModification == 0) // skip
				continue;

			this->addOrReplaceColorMap(lookup.second, latestModification);
		} catch (std::exception & bad)
		{
			if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
				tissuestack::logging::TissueStackLogger::instance()->error(
					"Could not load color map from lookup file '%s' for the following reason:\n%s\n",
					lookup.second->getLabelLookupId().c_str(), bad.what());
		}
	}
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

const std::string tissuestack::imaging::TissueStackColorMapStore::getColorMapDirectory()
{
	std::string dir =
		tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory("colormaps_directory");
	if (dir.empty())
		dir = COLORMAP_PATH;

	return dir;
}

tissuestack::imaging::TissueStackColorMapStore * tissuestack::imaging::TissueStackColorMapStore::_instance = nullptr;
