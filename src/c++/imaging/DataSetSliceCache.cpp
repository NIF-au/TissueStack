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

tissuestack::imaging::DataSetSliceCache::~DataSetSliceCache()
{
	if (this->_cache == nullptr) return;

	for (unsigned long int i=0;i<this->_numberOfCachedSlices;i++)
		if (this->_cache[i])
			delete this->_cache[i];

	delete [] this->_cache;
}

tissuestack::imaging::DataSetSliceCache::DataSetSliceCache(const TissueStackRawData * image) : _numberOfCachedSlices(0), _cache(nullptr)
{
	if (image == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException,
			"image param is null");

	for (auto dim : image->getDimensionOrder())
		this->_numberOfCachedSlices += image->getDimensionByLongName(dim)->getNumberOfSlices();

	this->_cache = new tissuestack::imaging::SliceCacheEntry * [this->_numberOfCachedSlices];
	for (unsigned long int x=0;x<this->_numberOfCachedSlices;x++)
		this->_cache[x] = nullptr;
}

const unsigned long int tissuestack::imaging::DataSetSliceCache::getNumberOfCachedSlices() const
{
	return this->_numberOfCachedSlices;
}

const bool tissuestack::imaging::DataSetSliceCache::setSlice(
	const unsigned long int slice, tissuestack::imaging::SliceCacheEntry * cache_data)
{
	if (slice >= this->_numberOfCachedSlices || cache_data == nullptr)
		return false;

	if (this->_cache && this->_cache[slice])
		delete this->_cache[slice];

	this->_cache[slice] = cache_data;

	return true;
}

tissuestack::imaging::SliceCacheEntry * tissuestack::imaging::DataSetSliceCache::getSlice(const unsigned long int slice) const
{
	if (slice >= this->_numberOfCachedSlices) return nullptr;

	return this->_cache[slice];
}

const bool tissuestack::imaging::DataSetSliceCache::isSliceCached(const unsigned long int slice) const
{
	if (slice >= this->_numberOfCachedSlices)
		return false;

	return (this->_cache != nullptr && this->_cache[slice] != nullptr);
}

void tissuestack::imaging::DataSetSliceCache::eraseSlice(const unsigned long int slice)
{
	if (slice >= this->_numberOfCachedSlices) return;

	if (this->_cache[slice])
	{
		delete this->_cache[slice];
		this->_cache[slice] = nullptr;
	}
}
