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

void tissuestack::common::TissueStackProcessingStrategy::process()
{
	// delegate
	this->_default_strategy->process();
};

void tissuestack::common::TissueStackProcessingStrategy::stop()
{
	// delegate
	this->_default_strategy->stop();
};
