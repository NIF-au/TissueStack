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
		((*functionality)(this));
		this->setRunningFlag(false); // mark us as finished
		this->resetStopFlag(); // reset any stop flag that was raised
	}
}

void tissuestack::execution::SimpleSequentialExecution::stop()
{
	// these lines are mostly of a 'rhethorical' nature given sequential execution
	// but what the heck ... for completeness sake
	if (this->isRunning())
		this->raiseStopFlag();
}
