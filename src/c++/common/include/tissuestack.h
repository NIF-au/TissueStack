#ifndef	__TISSUESTACK_H__
#define __TISSUESTACK_H__

#include "exceptions.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

namespace tissuestack
{
	namespace common
	{

		class ProcessingStrategy
		{
			protected:
				ProcessingStrategy();
			public:
				ProcessingStrategy & operator=(const ProcessingStrategy&) = delete;
				ProcessingStrategy(const ProcessingStrategy&) = delete;
				virtual ~ProcessingStrategy();
				void virtual init() = 0;
				void virtual process() = 0;
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
				void process();
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

		class Request
		{
			protected:
				Request();
			public:
				virtual const std::string getContent() const = 0;
				virtual ~Request();
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
