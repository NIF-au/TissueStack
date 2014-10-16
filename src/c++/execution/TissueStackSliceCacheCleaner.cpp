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
#include "networking.h"
#include "imaging.h"
#include "services.h"
#include "execution.h"

tissuestack::execution::TissueStackSliceCacheCleaner::TissueStackSliceCacheCleaner() :
	tissuestack::execution::ThreadPool(1)
{
	tissuestack::logging::TissueStackLogger::instance()->info("Launching Slice Cache Cleaner");

	try
	{
		tissuestack::imaging::TissueStackSliceCache::instance();
	} catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Could not instantiate TissueStackSliceCache:\n%s\n", bad.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not instantiate the Slice Cache!");
	}
}

void tissuestack::execution::TissueStackSliceCacheCleaner::init()
{
	// the clean loop
	std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> clean_loop =
		[this] (tissuestack::execution::WorkerThread * assigned_worker)
		{
		tissuestack::logging::TissueStackLogger::instance()->info(
				"Slice Cache Cleaner Thread %u is ready\n",
				std::hash<std::thread::id>()(std::this_thread::get_id()));

			while (!this->isStopFlagRaised())
			{
				usleep(5000000); // 5,000,000 micro seconds /5 seconds

				if (this->hasNoTasksQueued())
					break;

				if (tissuestack::utils::System::getFreeRam() > tissuestack::imaging::TissueStackSliceCache::MINIMUM_FREE_RAM_IN_BYTES)
					continue;

				tissuestack::imaging::TissueStackSliceCache::instance()->cleanUpCache();
			}
			tissuestack::logging::TissueStackLogger::instance()->info(
					"Slice Cache Cleaner Thread %u is about to stop working!\n",
					std::hash<std::thread::id>()(std::this_thread::get_id()));
			assigned_worker->stop();
		};

	this->init0(clean_loop);
}

void tissuestack::execution::TissueStackSliceCacheCleaner::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	if (functionality)
		delete functionality;
}

void tissuestack::execution::TissueStackSliceCacheCleaner::addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	if (functionality)
		delete functionality;
}

const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * tissuestack::execution::TissueStackSliceCacheCleaner::removeTask()
{
	return nullptr;
}

bool tissuestack::execution::TissueStackSliceCacheCleaner::hasNoTasksQueued()
{
	return !tissuestack::imaging::TissueStackSliceCache::doesInstanceExist();
}

