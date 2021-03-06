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

tissuestack::common::RequestTimeStampStore::RequestTimeStampStore() {}

tissuestack::common::RequestTimeStampStore * tissuestack::common::RequestTimeStampStore::instance()
{
	if (tissuestack::common::RequestTimeStampStore::_instance == nullptr)
		tissuestack::common::RequestTimeStampStore::_instance = new tissuestack::common::RequestTimeStampStore();

	return tissuestack::common::RequestTimeStampStore::_instance;
}

const bool tissuestack::common::RequestTimeStampStore::doesInstanceExist()
{
	return (tissuestack::common::RequestTimeStampStore::_instance != nullptr);
}

void tissuestack::common::RequestTimeStampStore::purgeInstance()
{
	delete tissuestack::common::RequestTimeStampStore::_instance;
	tissuestack::common::RequestTimeStampStore::_instance = nullptr;
}

const bool tissuestack::common::RequestTimeStampStore::checkForExpiredEntry(
		const unsigned long long int id, const unsigned long long int timestamp)
{
	if (id == 0 || timestamp == 0) return false;

	std::lock_guard<std::mutex> lock(this->_timestamp_mutex);

	// get existing value and compare it against the new value
	unsigned long long int old_difference = 0;

	try
	{
		old_difference = tissuestack::common::RequestTimeStampStore::_timestamps.at(id);
	}  catch (const std::out_of_range& key_does_not_exist)
	{
		tissuestack::common::RequestTimeStampStore::_timestamps[id] =
			this->calculateTimeDifference(id, timestamp);
		return false;
	}

	const unsigned long long int new_difference = this->calculateTimeDifference(id, timestamp);
	if (new_difference < old_difference)
		return true;

	// update timestamp
	if (new_difference > old_difference)
		tissuestack::common::RequestTimeStampStore::_timestamps[id] =
				new_difference;

	return false;
}

const bool tissuestack::common::RequestTimeStampStore::doesIdExist(const unsigned long long int id)
{
	std::lock_guard<std::mutex> lock(this->_timestamp_mutex);

	try
	{
		tissuestack::common::RequestTimeStampStore::_timestamps.at(id);
		return true;
	}  catch (const std::out_of_range& key_does_not_exist)
	{
		return false;
	}
}

inline const unsigned long long int tissuestack::common::RequestTimeStampStore::calculateTimeDifference(
		unsigned long long int id, unsigned long long int timestamp)
{
	// the id is longer by a few digits so we right pad the difference with 9s
	std::string timestampAsAString = std::to_string(timestamp);

	unsigned short numberOfIdDigits = std::to_string(id).length();
	unsigned short numberOfTimestampDigits = timestampAsAString.length();
	short deltaOfDigits = numberOfIdDigits-numberOfTimestampDigits;
	if (deltaOfDigits <= 0)
		return 0;

	char tmp[deltaOfDigits];
	memset(tmp, '9', deltaOfDigits);
	timestampAsAString.append(tmp, deltaOfDigits);

	unsigned long long int paddedTimeStamp = strtoull(timestampAsAString.c_str(), NULL, 10);
	// this is a stability measure against hand-crafted ids/timestamps
	// as well as rare asynchronous mishaps when reloading the tissuestack front end
	if (id > paddedTimeStamp)
		return 0;

	return paddedTimeStamp-id;
}

tissuestack::common::RequestTimeStampStore * tissuestack::common::RequestTimeStampStore::_instance = nullptr;
