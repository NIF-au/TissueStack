#include "execution.h"

tissuestack::execution::ThreadPool::ThreadPool(short number_of_threads) : _number_of_threads(number_of_threads) {}

short tissuestack::execution::ThreadPool::getNumberOfThreads() const
{
	return this->_number_of_threads;
}

void tissuestack::execution::ThreadPool::init()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;
}

void tissuestack::execution::ThreadPool::process(
		const std::function<void (const tissuestack::common::Request * request)> * functionality,
		const tissuestack::common::Request * request)
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;;
}

void tissuestack::execution::ThreadPool::stop()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;;
}
