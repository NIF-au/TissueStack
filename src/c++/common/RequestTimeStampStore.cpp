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
		return false;
	}

	const unsigned long long int new_difference = this->calculateTimeDifference(id, timestamp);
	if (new_difference < old_difference)
	{
		//tissuestack::logging::TissueStackLogger::instance()->debug(
		//		"EXPIRED: Id: %llu\tTime: %llu\tOld Delta:%llu\tNew Delta:%llu\n", id, timestamp, old_difference, new_difference);
		return true;
	}

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
	if (deltaOfDigits <= 0) return 0;

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

void tissuestack::common::RequestTimeStampStore::addTimeStamp(
		const unsigned long long int id,
		const unsigned long long int timestamp)
{
	if (id == 0 || timestamp == 0 || this->doesIdExist(id))
		return;

	std::lock_guard<std::mutex> lock(this->_timestamp_mutex);

	// let's add the new diff
	const unsigned long long int diff = this->calculateTimeDifference(id, timestamp);

	//tissuestack::logging::TissueStackLogger::instance()->debug(
	//		"ADDED: Id: %llu\tTime: %llu\tDelta:%llu\n", id, timestamp, diff);

	// perhaps we need to clear the hash map
	if (tissuestack::common::RequestTimeStampStore::_timestamps.size() >= tissuestack::common::RequestTimeStampStore::MAX_ENTRIES)
		tissuestack::common::RequestTimeStampStore::_timestamps.clear();

	tissuestack::common::RequestTimeStampStore::_timestamps[id] = diff;
}

tissuestack::common::RequestTimeStampStore * tissuestack::common::RequestTimeStampStore::_instance = nullptr;
