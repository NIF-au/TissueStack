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
#include "database.h"
#include "networking.h"
#include "imaging.h"

tissuestack::imaging::TissueStackLabelLookupStore::TissueStackLabelLookupStore()
{
	this->updateLabelLookupStore(true);
}

void tissuestack::imaging::TissueStackLabelLookupStore::updateLabelLookupStore(bool initial)
{
	const std::string dir = tissuestack::imaging::TissueStackLabelLookupStore::getLabelLookupDirectory();
	if (!tissuestack::utils::System::directoryExists(dir) &&
		!tissuestack::utils::System::createDirectory(dir, 0755))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create label lookup directory!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(dir);
	for (std::string f : fileList)
	{
		try
		{
			if (f.rfind("/.") != std::string::npos) // skip .files
				continue;

			tissuestack::imaging::TissueStackLabelLookup * newLabelLookup =
				const_cast<tissuestack::imaging::TissueStackLabelLookup *>(
					initial ? // first load (from directory)
						tissuestack::imaging::TissueStackLabelLookup::fromFile(f) :
						this->findLabelLookupByFullPath(f)); // update of already loaded file

			time_t lastModificationTime =
				newLabelLookup && newLabelLookup->getLastModified() > 0 ?
					newLabelLookup->getLastModified() :
					tissuestack::utils::System::getLastModifiedTime(f);

			if (initial) // we set the last modified for the inital load minus 1 to force addition
				lastModificationTime -= 1;
			else if (!initial && newLabelLookup == nullptr) // this happens if a new file has been added
			{
				tissuestack::logging::TissueStackLogger::instance()->info(
					"Adding new lookup file '%s'", f.c_str());

				newLabelLookup =
					const_cast<tissuestack::imaging::TissueStackLabelLookup *>(
					tissuestack::imaging::TissueStackLabelLookup::fromFile(f));
				lastModificationTime -= 1;
			}

			// do the time comparison and decide to not update if there was no file change
			const time_t latestModification =
				tissuestack::utils::System::hasFileBeenModifiedSince(f, lastModificationTime);

			if (latestModification == 0) // skip
				continue;

			newLabelLookup->setUpdateFlag(true);
			if (!initial) newLabelLookup->updateLabelLookup(f);
			this->addOrReplaceLabelLookup(newLabelLookup);
			this->synchronizeLabelLookupWithDataBase(newLabelLookup);
			newLabelLookup->setUpdateFlag(false);
			newLabelLookup->setLastModified( // we set the last modified for added files but deduct 1 so that we force addition
					latestModification);
		} catch (std::exception & bad)
		{
			if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
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
	{
		if (l.second->getLabelLookupId(true).compare(id) == 0)
			return l.second;
	}

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
	{
		entry->second->dumpLabelLookupToDebugLog();
	}
 }

void tissuestack::imaging::TissueStackLabelLookupStore::synchronizeLabelLookupWithDataBase(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	std::unique_ptr<const tissuestack::imaging::TissueStackLabelLookup> hit(
			tissuestack::database::LabelLookupDataProvider::queryLookupValuesByFileName(labelLookup->getLabelLookupId(true)));

	if (hit.get() == nullptr) // persist if not exists
		tissuestack::database::LabelLookupDataProvider::persistLookupValues(labelLookup);
	else // update
		tissuestack::database::LabelLookupDataProvider::updateLookupValues(hit.get(), labelLookup);
}

const std::string tissuestack::imaging::TissueStackLabelLookupStore::getLabelLookupDirectory()
{
	std::string dir =
		tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory("lookup_directory");
	if (dir.empty())
		dir = LABEL_LOOKUP_PATH;

	return dir;
}

tissuestack::imaging::TissueStackLabelLookupStore * tissuestack::imaging::TissueStackLabelLookupStore::_instance = nullptr;
