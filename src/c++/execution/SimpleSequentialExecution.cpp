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
#include "execution.h"

tissuestack::execution::SimpleSequentialExecution::SimpleSequentialExecution() {}

void tissuestack::execution::SimpleSequentialExecution::init()
{
	// empty for now
}

void tissuestack::execution::SimpleSequentialExecution::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// delegate only if we haven't received a null pointer for a closure
	// AND if the stop flag is down
	if (!this->isStopFlagRaised() && functionality)
	{
		this->setRunningFlag(true); // mark us as running
		try
		{
			((*functionality)(this));
			this->setRunningFlag(false); // mark us as finished
			this->resetStopFlag(); // reset any stop flag that was raised

			// clean up pointer
			delete functionality;
		}  catch (std::exception& bad)
		{
			this->setRunningFlag(false); // mark us as finished
			this->resetStopFlag(); // reset any stop flag that was raised
			// clean up and propagate
			delete functionality;
			throw bad;
		}
	}
}

void tissuestack::execution::SimpleSequentialExecution::stop()
{
	// these lines are mostly of a 'rhethorical' nature given sequential execution
	// but what the heck ... for completeness sake
	if (this->isRunning())
		this->raiseStopFlag();
}
