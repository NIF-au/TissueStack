#include "tissuestack.h"

tissuestack::common::RequestTimeStampStore::RequestTimeStampStore() {}

tissuestack::common::RequestTimeStampStore * tissuestack::common::RequestTimeStampStore::instance()
{
	if (tissuestack::common::RequestTimeStampStore::_instance == nullptr)
		tissuestack::common::RequestTimeStampStore::_instance = new tissuestack::common::RequestTimeStampStore();

	return tissuestack::common::RequestTimeStampStore::_instance;
}

bool tissuestack::common::RequestTimeStampStore::checkForExpiredEntry(unsigned long long int key, unsigned long long int value)
{
	if (key == 0) return false;

	bool ret = false;
	std::mutex mutex;
	mutex.lock();

	try
	{
		try
		{
			// get existing value and compare it against the old value (if exists)
			unsigned long long int old_value = tissuestack::common::RequestTimeStampStore::_timestamps.at(key);
			if (old_value > value) ret = true;
		}  catch (const std::out_of_range& key_does_not_exist) {}

		// let's add the new key/value if not expired
		if (!ret)
		{
			// perhaps we need to clear the hash map
			if (tissuestack::common::RequestTimeStampStore::_timestamps.size() >= tissuestack::common::RequestTimeStampStore::MAX_ENTRIES)
				tissuestack::common::RequestTimeStampStore::_timestamps.clear();
			tissuestack::common::RequestTimeStampStore::_timestamps[key] = value;
		}
	}
	catch (...)
	{
		// probably paranoia but never say never

	}
	mutex.unlock();
	return ret;
}

tissuestack::common::RequestTimeStampStore * tissuestack::common::RequestTimeStampStore::_instance = nullptr;
std::unordered_map<unsigned long long int, unsigned long long int> tissuestack::common::RequestTimeStampStore::_timestamps;

