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
	_task_queue_executor(new tissuestack::execution::TissueStackTaskQueueExecutor()),
	_slice_cache_cleaner(new tissuestack::execution::TissueStackSliceCacheCleaner()),
	_colormap_lookup_updater(new tissuestack::execution::TissueStackColorMapAndLookupUpdater())
{
	unsigned int cores = tissuestack::utils::System::getNumberOfCores();

	// heck let's be greedy
	short numberOfThreads = 5;
	if (cores > 2 && cores <= 5)
		numberOfThreads = 10;
	else if (cores > 5 && cores <= 10)
		numberOfThreads = 15;
	else if (cores > 10)
		numberOfThreads = 20;

	this->_default_strategy = new tissuestack::execution::ThreadPool(numberOfThreads);
};

tissuestack::common::TissueStackProcessingStrategy::~TissueStackProcessingStrategy()
{
	delete this->_default_strategy;
	delete this->_task_queue_executor;
	delete this->_slice_cache_cleaner;
	delete this->_colormap_lookup_updater;
};

void tissuestack::common::TissueStackProcessingStrategy::init()
{
	// delegate
	this->_default_strategy->init();
	this->_task_queue_executor->init();
	this->_slice_cache_cleaner->init();
	this->_colormap_lookup_updater->init();
	if (this->_default_strategy->isRunning() &&
			this->_task_queue_executor->isRunning() &&
			this->_slice_cache_cleaner->isRunning() &&
			this->_colormap_lookup_updater->isRunning())
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
	if (this->_slice_cache_cleaner->isRunning())
		this->_slice_cache_cleaner->stop();
	if (this->_colormap_lookup_updater->isRunning())
		this->_colormap_lookup_updater->stop();

	if (!this->_default_strategy->isRunning() &&
			!this->_task_queue_executor->isRunning() &&
			!this->_slice_cache_cleaner->isRunning() &&
			!this->_colormap_lookup_updater->isRunning())
			this->setRunningFlag(false);
};
