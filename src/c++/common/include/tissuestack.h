#ifndef	__TISSUESTACK_H__
#define __TISSUESTACK_H__

#include "exceptions.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

namespace tissuestack
{
	namespace common
	{
		class Request
		{
			public:
				enum class Type
				{
					RAW_HTTP_REQUEST,
					HTTP_REQUEST,
					TS_IMAGE,
					TS_TILING,
					TS_CONVERSION
				};

				virtual const std::string * const getContent() const = 0;
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
			public:
				ProcessingStrategy & operator=(const ProcessingStrategy&) = delete;
				ProcessingStrategy(const ProcessingStrategy&) = delete;
				virtual ~ProcessingStrategy();
				void virtual init() = 0;
				void virtual process(const std::function<void (const Request * request)> * functionality, const Request * request) = 0;
				void virtual stop() = 0;
		};

		class TissueStackProcessingStrategy : public ProcessingStrategy
		{
			public:
				TissueStackProcessingStrategy & operator=(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy(const TissueStackProcessingStrategy&) = delete;
				TissueStackProcessingStrategy();
				~TissueStackProcessingStrategy();
				void init();
				void process(const std::function<void (const Request * request)> * functionality, const Request * request);
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
				static const RequestProcessor<ProcessorImplementation> * const instance(ProcessorImplementation * impl)
				{
					if (RequestProcessor<ProcessorImplementation>::_instance == nullptr)
						RequestProcessor<ProcessorImplementation>::_instance = new RequestProcessor<ProcessorImplementation>(impl);

					return const_cast<const RequestProcessor<ProcessorImplementation> * const>(RequestProcessor<ProcessorImplementation>::_instance);
				};
				void init() const
				{
					this->_impl->init();
				};
				void process() const {};
				void stop() const {};
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
