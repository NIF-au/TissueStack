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
		if (processing_strategy == nullptr || pre_tiling_task == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"PreTiling requires an instance of a processing strategy and a pre tiling task!");

		tissuestack::logging::TissueStackLogger::instance()->info(
			"Starting Tiling: %s => %s",
			pre_tiling_task->getInputImageData()->getFileName().c_str(),
			pre_tiling_task->getParametersForTaskFile().c_str());



		// TODO: implement pretiling
		// regularly check for shutdown/task status (cancellation)
		// make use of uncached image creation logic
		const bool finished =
			const_cast<tissuestack::services::TissueStackTilingTask *>(pre_tiling_task)->incrementSlicesDone();
		tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(pre_tiling_task->getId());




		tissuestack::logging::TissueStackLogger::instance()->info(
			"Finished Tiling: %s => %s",
			pre_tiling_task->getInputImageData()->getFileName().c_str(),
			pre_tiling_task->getParametersForTaskFile().c_str());


		// we take care of things in the online/server version
		if (finished && processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_pretiling_task.release()->getId());
	} catch (const std::exception & bad)
	{
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
