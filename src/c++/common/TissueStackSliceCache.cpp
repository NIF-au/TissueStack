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
#include "tissuestack.h"

const unsigned long long int tissuestack::common::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES = 500 * 1000 * 1024;

tissuestack::common::TissueStackSliceCache::~TissueStackSliceCache()
{
	std::lock_guard<std::mutex> lock(this->_cache_mutex);
	this->_is_being_cleaned = true;

	// loop over all data sets and cache entries and delete them
	for (auto cached_dataset : this->_cache_entries)
		for (auto cached_dimension : cached_dataset.second)
			for (auto cached_slice : cached_dimension.second)
				delete cached_slice.second;
}

tissuestack::common::TissueStackSliceCache::TissueStackSliceCache() : _is_being_cleaned(false)
{

}

tissuestack::common::TissueStackSliceCache * tissuestack::common::TissueStackSliceCache::instance()
{
	if (tissuestack::common::TissueStackSliceCache::_instance == nullptr)
		tissuestack::common::TissueStackSliceCache::_instance = new tissuestack::common::TissueStackSliceCache();

	return tissuestack::common::TissueStackSliceCache::_instance;
}

const bool tissuestack::common::TissueStackSliceCache::doesInstanceExist()
{
	return (tissuestack::common::TissueStackSliceCache::_instance != nullptr);
}

void tissuestack::common::TissueStackSliceCache::purgeInstance()
{
	delete tissuestack::common::TissueStackSliceCache::_instance;
	tissuestack::common::TissueStackSliceCache::_instance = nullptr;
}

const bool tissuestack::common::TissueStackSliceCache::addCacheEntry(
	const std::string dataset, const std::string dimension, const unsigned long long int slice, const unsigned char * data)
{
	if (this->_is_being_cleaned || dataset.empty() || dimension.empty() || data == nullptr)
		return false;

	std::lock_guard<std::mutex> lock(this->_cache_mutex);
	if (tissuestack::utils::System::getFreeRam() < tissuestack::common::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES)
	{
		this->cleanUpCache();
		return false;
	}

	this->_cache_entries[dataset][dimension][slice] = new tissuestack::common::SliceCacheEntry(data);

	return true;
}

const unsigned char *  tissuestack::common::TissueStackSliceCache::findCacheEntry(
	const std::string dataset, const std::string dimension, const unsigned long long int slice)
{
	if (this->_is_being_cleaned || dataset.empty() || dimension.empty())
		return false;

	std::lock_guard<std::mutex> lock(this->_cache_mutex);

	try
	{
		// look up data set
		auto cached_dataset = this->_cache_entries.at(dataset);
		auto cached_dimension = cached_dataset.at(dimension);
		tissuestack::common::SliceCacheEntry * cached_slice = cached_dimension.at(slice);
		if (cached_slice == nullptr) return nullptr;

		return cached_slice->getCacheData();
	} catch (std::out_of_range & not_found) {
		return nullptr;
	}

}

void tissuestack::common::TissueStackSliceCache::cleanUpCache()
{
	this->_is_being_cleaned = true;
	// TODO: implement
	this->_is_being_cleaned = false;
}

const bool tissuestack::common::TissueStackSliceCache::isBeingCleanedUp() const
{
	return this->_is_being_cleaned;
}

tissuestack::common::TissueStackSliceCache * tissuestack::common::TissueStackSliceCache::_instance = nullptr;
