#include "networking.h"
#include "imaging.h"
#include "services.h"

tissuestack::imaging::PreTiler::PreTiler() : _extractor(new tissuestack::imaging::UncachedImageExtraction()) {}

tissuestack::imaging::PreTiler::~PreTiler()
{
	if (this->_extractor)
		delete _extractor;
}

void tissuestack::imaging::PreTiler::preTile(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackTilingTask * pre_tiling_task)
{
	// this will be useful for offline tools to not need to keep track of pointer
	std::unique_ptr<const tissuestack::services::TissueStackTilingTask> ptr_pretiling_task(pre_tiling_task);

	try
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		const std::string fileName = pre_tiling_task->getInputImageData()->getFileName();
		const std::string params = pre_tiling_task->getParametersForTaskFile();
		tissuestack::logging::TissueStackLogger::instance()->info(
			"Starting Tiling: %s => %s",
			fileName.c_str(),
			params.c_str());

		// TODO: implement pretiling with separate dimension/zoom level loop function (inlined)
		// regularly check for shutdown/task status (cancellation)
		// make use of uncached image creation logic

		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		const bool finished =
			const_cast<tissuestack::services::TissueStackTilingTask *>(pre_tiling_task)->incrementSlicesDone();
		tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(pre_tiling_task->getId());

		sleep(10);
		ptr_pretiling_task.release();


		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		// we take care of things in the online/server version
		if (finished && processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_pretiling_task.release()->getId());

		tissuestack::logging::TissueStackLogger::instance()->info(
			"Finished Tiling: %s => %s",
			fileName.c_str(),
			params.c_str());
	} catch (const std::exception & bad)
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to pre-tile: %s => %s",
			pre_tiling_task->getInputImageData()->getFileName().c_str(),
			pre_tiling_task->getParametersForTaskFile().c_str());

		// flag task as error
		if (processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsErroneous(
				ptr_pretiling_task.release()->getId());
	}
}

const bool tissuestack::imaging::PreTiler::hasBeenCancelledOrShutDown(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	std::unique_ptr<const tissuestack::services::TissueStackTilingTask> & ptr_pretiling_task) const
{
	if (processing_strategy == nullptr)
		return true;

	// abortion check
	if (!processing_strategy->isRunning()
			|| processing_strategy->isStopFlagRaised())
	{
		if (processing_strategy->isOnlineStrategy() && ptr_pretiling_task)
			ptr_pretiling_task.release();

		return true;
	}

	// cancel check
	if (ptr_pretiling_task.get() == nullptr
		||	(ptr_pretiling_task &&
				(ptr_pretiling_task->getStatus() ==
						tissuestack::services::TissueStackTaskStatus::CANCELLED
					|| ptr_pretiling_task->getStatus() ==
							tissuestack::services::TissueStackTaskStatus::ERRONEOUS)))
		return true;

	return false;
}


