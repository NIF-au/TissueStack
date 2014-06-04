#include "execution.h"

tissuestack::common::TissueStackProcessingStrategy::TissueStackProcessingStrategy() : _default_strategy(nullptr)
{
	//TODO: choose based on number of cores available
	this->_default_strategy = new tissuestack::execution::ThreadPool(static_cast<short>(10));
	//this->_default_strategy = new tissuestack::execution::SimpleSequentialExecution();
};

tissuestack::common::TissueStackProcessingStrategy::~TissueStackProcessingStrategy()
{
	delete this->_default_strategy;
};

void tissuestack::common::TissueStackProcessingStrategy::init()
{
	// delegate
	this->_default_strategy->init();
	if (this->_default_strategy->isRunning())
		this->setRunningFlag(true);
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
	if (this->_default_strategy->isRunning())
	{
		this->_default_strategy->stop();
		if (!this->_default_strategy->isRunning())
			this->setRunningFlag(false);
	}
};
