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

const unsigned long long int tissuestack::imaging::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES = 500 * 1000 * 1024;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::SECOND_IN_MILLIS = 1000;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::MINUTE_IN_MILLIS =
	tissuestack::imaging::TissueStackSliceCache::SECOND_IN_MILLIS * 60;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::HOUR_IN_MILLIS =
	tissuestack::imaging::TissueStackSliceCache::MINUTE_IN_MILLIS * 60;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::DAY_IN_MILLIS =
	tissuestack::imaging::TissueStackSliceCache::HOUR_IN_MILLIS * 24;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::WEEK_IN_MILLIS =
	tissuestack::imaging::TissueStackSliceCache::DAY_IN_MILLIS * 7;
const unsigned long long int tissuestack::imaging::TissueStackSliceCache::MONTH_IN_MILLIS =
	tissuestack::imaging::TissueStackSliceCache::WEEK_IN_MILLIS * 30;

tissuestack::imaging::TissueStackSliceCache::~TissueStackSliceCache()
{
	std::lock_guard<std::mutex> lock(this->_cache_mutex);
	this->_is_being_cleaned = true;

	// loop over all data sets and cache entries and delete them
	for (auto cached_dataset : this->_cache)
		if (cached_dataset.second) delete cached_dataset.second;

	this->_is_being_cleaned = false;
}

tissuestack::imaging::TissueStackSliceCache::TissueStackSliceCache() : _is_being_cleaned(false), _is_empty(true)
{
	// we take the existing data sets and build up a cache structure
	if (!tissuestack::imaging::TissueStackDataSetStore::doesInstanceExist())
		return;

	this->_is_being_cleaned = true;

	// we take the existing data sets and build up a cache structure
	const std::vector<const tissuestack::imaging::TissueStackRawData * > dataSets =
		tissuestack::imaging::TissueStackDataSetStore::instance()->getDataSetList();

	// initialize cache
	for (auto ds : dataSets)
		this->_cache[ds->getFileName()] = new tissuestack::imaging::DataSetSliceCache(ds);

	this->_is_being_cleaned = false;
}

tissuestack::imaging::TissueStackSliceCache * tissuestack::imaging::TissueStackSliceCache::instance()
{
	if (tissuestack::imaging::TissueStackSliceCache::_instance == nullptr)
		tissuestack::imaging::TissueStackSliceCache::_instance = new tissuestack::imaging::TissueStackSliceCache();

	return tissuestack::imaging::TissueStackSliceCache::_instance;
}

const bool tissuestack::imaging::TissueStackSliceCache::doesInstanceExist()
{
	return (tissuestack::imaging::TissueStackSliceCache::_instance != nullptr);
}

void tissuestack::imaging::TissueStackSliceCache::purgeInstance()
{
	delete tissuestack::imaging::TissueStackSliceCache::_instance;
	tissuestack::imaging::TissueStackSliceCache::_instance = nullptr;
}

const bool tissuestack::imaging::TissueStackSliceCache::addCacheEntry(
	const std::string dataset, const unsigned long int slice, const unsigned char * data)
{
	if (this->isBeingCleanedUp() || dataset.empty() || data == nullptr)
			return false;

	if (tissuestack::utils::System::getFreeRam() < tissuestack::imaging::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES)
		return false;

	std::lock_guard<std::mutex> lock(this->_cache_mutex);

	bool oldStatus = this->_is_empty;

	try
	{
		this->_is_empty = false;
		tissuestack::imaging::DataSetSliceCache * cache = this->_cache.at(dataset);
		return cache->setSlice(slice, new tissuestack::imaging::SliceCacheEntry(data));
	} catch (std::out_of_range & not_found) {
		// we did not have this data set before => add it to cache structure
		try
		{
			const tissuestack::imaging::TissueStackDataSet * ds =
				tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(dataset);
			if (ds == nullptr || ds->getImageData() == nullptr || !ds->getImageData()->isRaw()) return false;

			this->_cache[ds->getDataSetId()] =
				new tissuestack::imaging::DataSetSliceCache(
					static_cast<const tissuestack::imaging::TissueStackRawData *>(ds->getImageData()));
			return this->_cache[ds->getDataSetId()]->setSlice(slice, new tissuestack::imaging::SliceCacheEntry(data));
		} catch (std::exception & ex) {
			this->_is_empty = oldStatus;
			tissuestack::logging::TissueStackLogger::instance()->error(
					"Unable to add new dataset %s to cache: %s", dataset.c_str(), ex.what());
			return false;
		}
	}

	this->_is_empty = oldStatus;
	return false;
}

const unsigned char *  tissuestack::imaging::TissueStackSliceCache::findCacheEntry(
	const std::string dataset, const unsigned long int slice)
{
	if (this->isBeingCleanedUp() || dataset.empty() || this->_is_empty)
		return false;

	std::lock_guard<std::mutex> lock(this->_cache_mutex);

	try
	{
		// look up data set
		tissuestack::imaging::DataSetSliceCache * cache = this->_cache.at(dataset);
		tissuestack::imaging::SliceCacheEntry * cached_slice = cache->getSlice(slice);
		if (cached_slice == nullptr)
			return nullptr;

		return cached_slice->getCacheData();
	} catch (std::out_of_range & not_found) {
		return nullptr;
	}
}

void tissuestack::imaging::TissueStackSliceCache::cleanUpCache()
{
	if (this->isBeingCleanedUp() || this->_is_empty)
		return;

	std::lock_guard<std::mutex> lock(this->_cache_mutex);

	this->_is_being_cleaned = true;

	unsigned long int count = 0;

	// let's use this strategy for now: access time loop down to 5 minutes
	// with repeated checks whether we bring the free ram back up to exceed our threshold
	unsigned long long int ACCESS_TIMES[] =
	{
		tissuestack::imaging::TissueStackSliceCache::MONTH_IN_MILLIS,
		tissuestack::imaging::TissueStackSliceCache::WEEK_IN_MILLIS,
		tissuestack::imaging::TissueStackSliceCache::DAY_IN_MILLIS,
		tissuestack::imaging::TissueStackSliceCache::HOUR_IN_MILLIS,
		tissuestack::imaging::TissueStackSliceCache::MINUTE_IN_MILLIS * 5
	};

	const unsigned long long int NOW = tissuestack::utils::System::getSystemTimeInMillis();

	for (auto t : ACCESS_TIMES)
	{
		for (auto cached_dataset : this->_cache)
		{
			tissuestack::imaging::DataSetSliceCache * cache = cached_dataset.second;
			for (unsigned int x=0;x<cache->getNumberOfCachedSlices();x++)
				if (cache->isSliceCached(x) && (NOW - cache->getSlice(x)->getTimeStampForLastAccess() > t))
				{
					cache->eraseSlice(x);
					count++;
				}
		}

		if (tissuestack::utils::System::getFreeRam() > tissuestack::imaging::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES)
		{
			if (count > 0)
				tissuestack::logging::TissueStackLogger::instance()->info("Freed %lu cache entries.", count);

			this->_is_being_cleaned = false;
			return;
		}
	}

	double ACCESS_PERCENTAGES[] =
	{
		0.5, // 50 % of most accessed slice
		0.4, // 40 % of most accessed slice
		0.3, // etc.
		0.2,
		0.1
	};

	// should the above strategy fail to free sufficient memory the slice cache access numbers become the crucial criterion
	// we do this over each data set separately and loop in percentages of highest access number
	for (auto p : ACCESS_PERCENTAGES)
	{
		for (auto cached_dataset : this->_cache)
		{
			tissuestack::imaging::DataSetSliceCache * cache = cached_dataset.second;

			// this loop determines the most frequently accessed entry
			unsigned long long int highest_access_count = 0;
			for (unsigned int x=0;x<cache->getNumberOfCachedSlices();x++)
				if (cache->isSliceCached(x) && cache->getSlice(x)->getAccessCount() > highest_access_count)
					highest_access_count = cache->getSlice(x)->getAccessCount();

			// now delete those entries that are below the percentage
			unsigned long long int threshold =
				static_cast<unsigned long long int>(static_cast<double>(highest_access_count) * p);

			if (threshold == 0)
				continue;

			for (unsigned int x=0;x<cache->getNumberOfCachedSlices();x++)
				if (cache->isSliceCached(x) && cache->getSlice(x)->getAccessCount() < threshold &&
						(NOW - cache->getSlice(x)->getTimeStampForLastAccess() > 10000))
				{
					cache->eraseSlice(x);
					count++;
				}
		}

		// we've free enough, let's leave ...
		if (tissuestack::utils::System::getFreeRam() > tissuestack::imaging::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES)
		{
			if (count > 0)
				tissuestack::logging::TissueStackLogger::instance()->info("Freed %lu cache entries.", count);

			this->_is_being_cleaned = false;
			return;
		}
	}

	// still not good enough => empty entire cache
	for (auto cached_dataset : this->_cache)
	{
		tissuestack::imaging::DataSetSliceCache * cache = cached_dataset.second;
		for (unsigned int x=0;x<cache->getNumberOfCachedSlices();x++)
			if (cache->isSliceCached(x))
			{
				cache->eraseSlice(x);
				count++;
			}
	}
	this->_is_empty = true;

	if (count > 0)
		tissuestack::logging::TissueStackLogger::instance()->info("Freed %lu cache entries.", count);

	this->_is_being_cleaned = false;
}

const bool tissuestack::imaging::TissueStackSliceCache::isBeingCleanedUp() const
{
	return this->_is_being_cleaned;
}

tissuestack::imaging::TissueStackSliceCache * tissuestack::imaging::TissueStackSliceCache::_instance = nullptr;
