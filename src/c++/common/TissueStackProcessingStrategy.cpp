#include "tissuestack.h"
#include "execution.h"

tissuestack::common::TissueStackProcessingStrategy::TissueStackProcessingStrategy() : _default_strategy(nullptr)
{
	this->_default_strategy = new tissuestack::execution::ThreadPool(static_cast<short>(10));
};

tissuestack::common::TissueStackProcessingStrategy::~TissueStackProcessingStrategy()
{
	delete this->_default_strategy;
};

void tissuestack::common::TissueStackProcessingStrategy::init()
{
	// delegate
	this->_default_strategy->init();
};

void tissuestack::common::TissueStackProcessingStrategy::process(
		const std::function<void (const tissuestack::common::Request * request)> * functionality,
		const tissuestack::common::Request * request)
{
	// delegate
	this->_default_strategy->process(nullptr, nullptr);
};

void tissuestack::common::TissueStackProcessingStrategy::stop()
{
	// delegate
	this->_default_strategy->stop();
};
