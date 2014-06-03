#include "execution.h"

tissuestack::common::TissueStackProcessingStrategy::TissueStackProcessingStrategy() : _default_strategy(nullptr)
{
	//this->_default_strategy = new tissuestack::execution::ThreadPool(static_cast<short>(10));
	this->_default_strategy = new tissuestack::execution::SimpleSequentialExecution();
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
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// delegate
	this->_default_strategy->process(functionality);
};

void tissuestack::common::TissueStackProcessingStrategy::stop()
{
	// delegate
	this->_default_strategy->stop();
};
