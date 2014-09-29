/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
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
