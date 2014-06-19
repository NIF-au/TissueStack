#include "execution.h"

tissuestack::common::TissueStackProcessingStrategy::TissueStackProcessingStrategy() : _default_strategy(nullptr)
{
	// depending on the number of cores, we use 75% rounded up
	if (tissuestack::utils::System::getNumberOfCores() > 1)
	{
		this->_default_strategy = new tissuestack::execution::ThreadPool(
				static_cast<short>(ceil(tissuestack::utils::System::getNumberOfCores() * 0.75)));
	} else // or sequentially execute with only one meager core
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
