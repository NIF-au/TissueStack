#include "tissuestack.h"

tissuestack::common::ProcessingStrategy::ProcessingStrategy()
{
	// abstract
}

tissuestack::common::ProcessingStrategy::~ProcessingStrategy()
{
	//abstract
}

void tissuestack::common::ProcessingStrategy::setRunningFlag(bool isRunning)
{
	this->_isRunning = isRunning;
}

bool tissuestack::common::ProcessingStrategy::isRunning() const
{
	return this->_isRunning;
}

bool tissuestack::common::ProcessingStrategy::isStopFlagRaised() const
{
	return this->_stopFlagRaised;
}

void tissuestack::common::ProcessingStrategy::raiseStopFlag()
{
	this->_stopFlagRaised = true;
}

void tissuestack::common::ProcessingStrategy::resetStopFlag()
{
	this->_stopFlagRaised = false;
}
