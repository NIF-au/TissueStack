#ifndef	__TISSUESTACK_H__
#define __TISSUESTACK_H__

#include "exceptions.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace tissuestack
{
	namespace common
	{
		class TimeStampHashMap final
		{
			public:
				static const unsigned int MAX_ENTRIES = 1000;
				TimeStampHashMap & operator=(const TimeStampHashMap&) = delete;
				TimeStampHashMap(const TimeStampHashMap&) = delete;
				static bool checkForExpiredEntry(unsigned long long int key, unsigned long long int value)
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
							unsigned long long int old_value = TimeStampHashMap::_timestamps.at(key);
							if (old_value > value) ret = true;
						}  catch (const std::out_of_range& key_does_not_exist) {}

						// let's add the new key/value if not expired
						if (!ret)
						{
							// perhaps we need to clear the hash map
							if (TimeStampHashMap::_timestamps.size() >= TimeStampHashMap::MAX_ENTRIES)
								TimeStampHashMap::_timestamps.clear();
							TimeStampHashMap::_timestamps[key] = value;
						}
					}
					catch (...)
					{
						// probably paranoia but never say never

					}
					mutex.unlock();
					return ret;
				}
			private:
				TimeStampHashMap();
				static std::unordered_map<unsigned long long int, unsigned long long int> _timestamps;
		};

		class Request
		{
			public:
				enum class Type
				{
					RAW_HTTP,
					HTTP,
					TS_IMAGE,
					TS_TILING,
					TS_CONVERSION
				};

				virtual const std::string getContent() const = 0;
				virtual const bool isObsolete() const = 0;
				virtual ~Request();
				const Request::Type getType() const;
			protected:
				Request();
				void setType(Request::Type type);
			private:
				Request::Type _type;
		};

		class ProcessingStrategy
		{
			protected:
				ProcessingStrategy();
				void setRunningFlag(bool isRunning);
				void raiseStopFlag();
				void resetStopFlag();
			public:
				ProcessingStrategy & operator=(const ProcessingStrategy&) = delete;
				ProcessingStrategy(const ProcessingStrategy&) = delete;
				virtual ~ProcessingStrategy();
				void virtual init() = 0;
				void virtual process(const std::function<void (const Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality, const Request * request) = 0;
				void virtual stop() = 0;
				bool isRunning() const;
				bool isStopFlagRaised() const;
			private:
				bool _stopFlagRaised = false;
				bool _isRunning = false;
		};

		class TissueStackProcessingStrategy : public ProcessingStrategy
		{
			public:
				TissueStackProcessingStrategy & operator=(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy();
				~TissueStackProcessingStrategy();
				void init();
				void process(const std::function<void (const Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality, const Request * request);
				void stop();
			private:
				ProcessingStrategy	* _default_strategy;
		};

		template<typename ProcessorImplementation>
		class RequestProcessor
		{
			public:
				RequestProcessor & operator=(const RequestProcessor&) = delete;
				RequestProcessor(const RequestProcessor&) = delete;
				~RequestProcessor() {
					delete this->_impl;
				};
				static const RequestProcessor<ProcessorImplementation> * instance(ProcessorImplementation * impl)
				{
					if (RequestProcessor<ProcessorImplementation>::_instance == nullptr)
						RequestProcessor<ProcessorImplementation>::_instance = new RequestProcessor<ProcessorImplementation>(impl);

					return const_cast<const RequestProcessor<ProcessorImplementation> * const>(RequestProcessor<ProcessorImplementation>::_instance);
				};
				void init() const
				{
					this->_impl->init();
				};
				void process(
						const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality,
						const tissuestack::common::Request * request) const
				{
					this->_impl->process(functionality, request);
				};
				void stop() const
				{
					this->_impl->stop();
				};
				bool isRunning() const
				{
					return this->_impl->isRunning();
				}
			private:
				static RequestProcessor<ProcessorImplementation> * _instance;
				ProcessorImplementation * _impl;
				RequestProcessor(ProcessorImplementation * impl)
				{
					this->_impl = impl;
				};
		};
		template<typename ProcessorImplementation>
		tissuestack::common::RequestProcessor<ProcessorImplementation> *
			tissuestack::common::RequestProcessor<ProcessorImplementation>::_instance =	nullptr;

		class RequestFilter
		{
			protected:
				RequestFilter();
			public:
				virtual ~RequestFilter();
				virtual const Request * const applyFilter(const Request * const) const = 0;
		};
	}
}

#endif	/* __TISSUESTACK_H__ */
