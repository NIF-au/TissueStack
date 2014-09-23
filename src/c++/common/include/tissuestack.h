#ifndef	__TISSUESTACK_H__
#define __TISSUESTACK_H__

#include "logging.h"
#include "exceptions.h"
#include "utils.h"
#include "parameters.h"

#include <magick/api.h>
#include <nifti1_io.h>
#include <minc2.h>

#include <math.h>

#include <string>
#include <cstring>

#include <array>
#include <unordered_map>
#include <queue>
#include <vector>

#include <mutex>
#include <memory>
#include <atomic>

#include <algorithm>
#include <functional>
#include <typeinfo>

namespace tissuestack
{
	namespace common
	{
		static const std::string NO_RESULTS_JSON = "{\"response\": {\"noResults\": \"No results found\"}}";
		static const unsigned int SOCKET_READ_BUFFER_SIZE = 2048;

		class RequestTimeStampStore final
		{
			public:

				RequestTimeStampStore & operator=(const RequestTimeStampStore&) = delete;
				RequestTimeStampStore(const RequestTimeStampStore&) = delete;

				static RequestTimeStampStore * instance();

				static const bool doesInstanceExist();
				void purgeInstance();

				const bool doesIdExist(const unsigned long long int id);

				const bool checkForExpiredEntry(
						const unsigned long long int id,
						const unsigned long long int timestamp);

				void addTimeStamp(
						const unsigned long long int id,
						const unsigned long long int timestamp);
			private:
				inline const unsigned long long int calculateTimeDifference(
						const unsigned long long int id,
						const unsigned long long int timestamp);

				static const unsigned int MAX_ENTRIES = 1000;
				RequestTimeStampStore();
				std::unordered_map<unsigned long long int, unsigned long long int> _timestamps;
				std::mutex _timestamp_mutex;
				static RequestTimeStampStore * _instance;
		};

		class Request
		{
			public:
				enum class Type
				{
					RAW_HTTP,
					HTTP,
					TS_IMAGE,
					TS_QUERY,
					TS_TILING,
					TS_CONVERSION,
					TS_SERVICES
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
				void setOfflineStrategyFlag();
				void raiseStopFlag();
				void resetStopFlag();
			public:
				ProcessingStrategy & operator=(const ProcessingStrategy&) = delete;
				ProcessingStrategy(const ProcessingStrategy&) = delete;
				virtual ~ProcessingStrategy();
				virtual void init() = 0;
				virtual void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality) = 0;
				virtual void stop() = 0;
				bool isRunning() const;
				bool isStopFlagRaised() const;
				void setRunningFlag(bool isRunning);
				virtual bool isOnlineStrategy() const;
			private:
				bool _stopFlagRaised = false;
				bool _isRunning = false;
				bool _isOnline = true;
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
						const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality) const
				{
					this->_impl->process(functionality);
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

		class TissueStackProcessingStrategy : public ProcessingStrategy
		{
			public:
				TissueStackProcessingStrategy & operator=(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy();
				~TissueStackProcessingStrategy();
				void init();
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				void stop();
			private:
				ProcessingStrategy	* _default_strategy;
				ProcessingStrategy * _task_queue_executor;
		};

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
