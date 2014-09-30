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

tissuestack::common::TissueStackProcessingStrategy::TissueStackProcessingStrategy() :
	_task_queue_executor(new tissuestack::execution::TissueStackTaskQueueExecutor())
{
	// from 2 cores we use 75% rounded up
	if (tissuestack::utils::System::getNumberOfCores() > 2)
	{
		this->_default_strategy = new tissuestack::execution::ThreadPool(
				static_cast<short>(ceil(tissuestack::utils::System::getNumberOfCores() * 0.75)));
	} else
		this->_default_strategy = new tissuestack::execution::ThreadPool(2);
};

tissuestack::common::TissueStackProcessingStrategy::~TissueStackProcessingStrategy()
{
	delete this->_default_strategy;
	delete this->_task_queue_executor;
};

void tissuestack::common::TissueStackProcessingStrategy::init()
{
	// delegate
	this->_default_strategy->init();
	this->_task_queue_executor->init();
	if (this->_default_strategy->isRunning() &&
			this->_task_queue_executor->isRunning())
		this->setRunningFlag(true);
};

void tissuestack::common::TissueStackProcessingStrategy::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// delegate
	this->_default_strategy->process(functionality);
};

void tissuestack::common::TissueStackProcessingStrategy::stop()
{
	// delegate
	if (this->_default_strategy->isRunning())
		this->_default_strategy->stop();
	if (this->_task_queue_executor->isRunning())
		this->_task_queue_executor->stop();

	if (!this->_default_strategy->isRunning() &&
			!this->_task_queue_executor->isRunning())
			this->setRunningFlag(false);
};
