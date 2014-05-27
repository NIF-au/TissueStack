#include "execution.h"

tissuestack::execution::ThreadPool::ThreadPool(short number_of_threads) : _number_of_threads(number_of_threads) {}

short tissuestack::execution::ThreadPool::getNumberOfThreads() const
{
	return this->_number_of_threads;
}

void tissuestack::execution::ThreadPool::init()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;
	//TODO: start worker threads

	// the thread pool is up and running
	if (!this->isStopFlagRaised())
		this->setRunningFlag(true);
}

void tissuestack::execution::ThreadPool::process(
		const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality,
		const tissuestack::common::Request * request)
{
	// dispatch functionality to the pool, only if we are running,
	// haven't received a stop flag and the closure is not null
	if (this->isRunning() && !this->isStopFlagRaised() && functionality)
	{
		//TODO: sleep vs wait vs conditional variable
		((*functionality)(request, this));
	}
}

void tissuestack::execution::ThreadPool::stop()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;;

	// raise stop flag to prevent new requests from being processed
	this->raiseStopFlag();

	// TODO: take down threads, one by one

	this->setRunningFlag(false);
}

