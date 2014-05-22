#include "execution.h"

tissuestack::execution::SimpleExecution::SimpleExecution() {}

void tissuestack::execution::SimpleExecution::init()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;
}

void tissuestack::execution::SimpleExecution::process(
		const std::function<void (const tissuestack::common::Request * request)> * functionality,
		const tissuestack::common::Request * request)
{
	if (functionality) ((*functionality)(request));
}

void tissuestack::execution::SimpleExecution::stop()
{
	std::cout << "Thread Pool Not implemented yet" << std::endl;;
}
