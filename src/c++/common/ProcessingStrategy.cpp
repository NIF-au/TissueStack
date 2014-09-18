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

void tissuestack::common::ProcessingStrategy::setOfflineStrategyFlag()
{
	this->_isOnline = false;
}


bool tissuestack::common::ProcessingStrategy::isRunning() const
{
	return this->_isRunning;
}

bool tissuestack::common::ProcessingStrategy::isStopFlagRaised() const
{
	return this->_stopFlagRaised;
}

bool tissuestack::common::ProcessingStrategy::isOnlineStrategy() const
{
	return this->_isOnline;
}

void tissuestack::common::ProcessingStrategy::raiseStopFlag()
{
	this->_stopFlagRaised = true;
}

void tissuestack::common::ProcessingStrategy::resetStopFlag()
{
	this->_stopFlagRaised = false;
}
