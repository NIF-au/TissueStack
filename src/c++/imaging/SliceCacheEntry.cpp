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

tissuestack::imaging::SliceCacheEntry::~SliceCacheEntry()
{
	if (this->_cache_data)
		delete [] this->_cache_data;
}

tissuestack::imaging::SliceCacheEntry::SliceCacheEntry(const unsigned char * cache_data) :
	_cache_data(cache_data), _timestamp_accessed(tissuestack::utils::System::getSystemTimeInMillis()), _access_count(0)
{}


const unsigned char * tissuestack::imaging::SliceCacheEntry::getCacheData()
{
	// every time this method is called we increment the access count
	// and update the last access timestamp
	const unsigned long long int MAX_UNSIGNED_LONG_LONG_INT =
		std::numeric_limits<unsigned long long int>::max();
	if (this->_access_count != MAX_UNSIGNED_LONG_LONG_INT)
		this->_access_count++;
	this->_timestamp_accessed = tissuestack::utils::System::getSystemTimeInMillis();

	return this->_cache_data;
}

const unsigned long long int tissuestack::imaging::SliceCacheEntry::getAccessCount() const
{
	return this->_access_count;
}

const unsigned long long int tissuestack::imaging::SliceCacheEntry::getTimeStampForLastAccess() const
{
	return this->_timestamp_accessed;
}

