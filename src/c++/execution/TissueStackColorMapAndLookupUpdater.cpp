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

tissuestack::execution::TissueStackColorMapAndLookupUpdater::TissueStackColorMapAndLookupUpdater() :
	tissuestack::execution::ThreadPool(1)
{
	tissuestack::logging::TissueStackLogger::instance()->info("Launching Color Map and Lookup Values Updater");

	try
	{
		tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist();
		tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist();
	} catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Color Map and Lookup Values Updater:\n%s\n", bad.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not instantiate the Color Map and Lookup Values Updater!");
	}
}

void tissuestack::execution::TissueStackColorMapAndLookupUpdater::init()
{
	// the update checker
	std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> clean_loop =
		[this] (tissuestack::execution::WorkerThread * assigned_worker)
		{
		tissuestack::logging::TissueStackLogger::instance()->info(
				"Color Map and Lookup Values Updater Thread %u is ready\n",
				std::hash<std::thread::id>()(std::this_thread::get_id()));

			while (!this->isStopFlagRaised())
			{
				usleep(5000000); // 5,000,000 micro seconds /5 seconds

				if (this->hasNoTasksQueued())
					break;

				// do synchronization of first label lookup, then colormap
				tissuestack::imaging::TissueStackLabelLookupStore::instance()->updateLabelLookupStore();
				tissuestack::imaging::TissueStackColorMapStore::instance()->updateColorMapStore();
			}
			tissuestack::logging::TissueStackLogger::instance()->info(
					"Color Map and Lookup Values Updater Thread %u is about to stop working!\n",
					std::hash<std::thread::id>()(std::this_thread::get_id()));
			assigned_worker->stop();
		};

	this->init0(clean_loop);
}

void tissuestack::execution::TissueStackColorMapAndLookupUpdater::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	if (functionality)
		delete functionality;
}

void tissuestack::execution::TissueStackColorMapAndLookupUpdater::addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	if (functionality)
		delete functionality;
}

const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * tissuestack::execution::TissueStackColorMapAndLookupUpdater::removeTask()
{
	return nullptr;
}

bool tissuestack::execution::TissueStackColorMapAndLookupUpdater::hasNoTasksQueued()
{
	return !tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist() || !tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist();
}

