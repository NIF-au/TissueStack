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
	if (!tissuestack::utils::System::directoryExists(LABEL_LOOKUP_PATH) &&
		!tissuestack::utils::System::createDirectory(LABEL_LOOKUP_PATH, 0755))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create label lookup directory!");

	const std::vector<std::string> fileList = tissuestack::utils::System::getFilesInDirectory(LABEL_LOOKUP_PATH);
	for (std::string f : fileList)
	{
		try
		{
			// TODO: skip . temporary files

			tissuestack::imaging::TissueStackLabelLookup * newLabelLookup =
				const_cast<tissuestack::imaging::TissueStackLabelLookup *>(
					initial ? // first load (from directory)
						tissuestack::imaging::TissueStackLabelLookup::fromFile(f) :
						this->findLabelLookupByFullPath(f)); // update of already loaded file

			if (!initial && newLabelLookup == nullptr) // this happens if a new file has been added
			{
				this->dumpAllLabelLookupsToDebugLog();

				tissuestack::logging::TissueStackLogger::instance()->info(
					"Adding file '%s'", f.c_str());

				newLabelLookup =
					const_cast<tissuestack::imaging::TissueStackLabelLookup *>(
					tissuestack::imaging::TissueStackLabelLookup::fromFile(f));
				newLabelLookup->setLastModified( // we set the last modified for added files but deduct 1 so that we force addition
					tissuestack::utils::System::getLastModifiedTime(f) - 1);
			}

			if (initial) // we set the last modified for the inital load minus 1 to force addition
			{
				// TODO: check time stamp for initial modification
				newLabelLookup->setLastModified(
					tissuestack::utils::System::getLastModifiedTime(f) - 1);
			}

			// do the time comparison and decide to not update if there was no file change
			const time_t latestModification =
				tissuestack::utils::System::hasFileBeenModifiedSince(f, newLabelLookup->getLastModified());

			if (latestModification == 0)
			{// skip
				tissuestack::logging::TissueStackLogger::instance()->info(
								"Skipping file '%s'", f.c_str());
				continue;
			}

			if (!initial)
				tissuestack::logging::TissueStackLogger::instance()->info(
					"Updating file '%s' after addition/modification", f.c_str());

			newLabelLookup->setUpdateFlag(true);
			this->addOrReplaceLabelLookup(newLabelLookup);
			this->synchronizeLabelLookupWithDataBase(newLabelLookup);
			newLabelLookup->setUpdateFlag(false);
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
		//entry->second->dumpLabelLookupToDebugLog();
		tissuestack::logging::TissueStackLogger::instance()->info(
				"Dumping Label Lookup: |%s|\n", entry->second->getLabelLookupId(true).c_str());
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

tissuestack::imaging::TissueStackLabelLookupStore * tissuestack::imaging::TissueStackLabelLookupStore::_instance = nullptr;
