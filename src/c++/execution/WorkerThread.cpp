#include "execution.h"

tissuestack::execution::WorkerThread::WorkerThread(
		std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop)
	: std::thread(wait_loop, this)
{
	this->_is_running = true;
}

bool tissuestack::execution::WorkerThread::isRunning() const
{
	return this->_is_running;
}

void tissuestack::execution::WorkerThread::stop()
{
	this->_is_running = false;
}
