#include "networking.h"
#include "imaging.h"
#include "services.h"

tissuestack::imaging::RawConverter::RawConverter(){}

void tissuestack::imaging::RawConverter::convert(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackConversionTask * converter_task)
{
	// this will be useful for offline tools to not need to keep track of pointer
	std::unique_ptr<const tissuestack::services::TissueStackConversionTask> ptr_converter_task(converter_task);

	try
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_converter_task))
			return;

		const std::string fileName = converter_task->getInputImageData()->getFileName();
		const std::string outFile = converter_task->getOutFile();

		tissuestack::logging::TissueStackLogger::instance()->info(
			"Staring Conversion: %s => %s",
			fileName.c_str(),
			outFile.c_str());



		// TODO: implement conversion with separate dimension and slice loop function (inlined)
		// regularly check for shutdown/task status (cancellation)
		// make 'visitor' methods for minc and nifti, e.g convert(MincImageData), convert(NiftiImageData)
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_converter_task))
			return;

		const bool finished =
			const_cast<tissuestack::services::TissueStackConversionTask *>(converter_task)->incrementSlicesDone();
		tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(converter_task->getId());






		// we take care of things in the online/server version
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_converter_task))
			return;

		if (finished && processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_converter_task.release()->getId());

		tissuestack::logging::TissueStackLogger::instance()->info(
			"Finished Conversion: %s => %s",
			fileName.c_str(),
			outFile.c_str());
	} catch (const std::exception & bad)
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_converter_task))
			return;

		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to convert: %s => %s",
			converter_task->getInputImageData()->getFileName().c_str(),
			converter_task->getOutFile().c_str());

		// we erase our partial work !
		unlink(converter_task->getOutFile().c_str());

		// flag task as error
		if (processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsErroneous(
				ptr_converter_task.release()->getId());
	}
}

const bool tissuestack::imaging::RawConverter::hasBeenCancelledOrShutDown(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	std::unique_ptr<const tissuestack::services::TissueStackConversionTask> & ptr_converter_task) const
{
	if (processing_strategy == nullptr)
		return true;

	// abortion check
	if (!processing_strategy->isRunning()
			|| processing_strategy->isStopFlagRaised())
	{
		if (processing_strategy->isOnlineStrategy() && ptr_converter_task)
			ptr_converter_task.release();

		return true;
	}

	// cancel check
	if (ptr_converter_task.get() == nullptr
		||	(ptr_converter_task &&
				(ptr_converter_task->getStatus() ==
						tissuestack::services::TissueStackTaskStatus::CANCELLED
					|| ptr_converter_task->getStatus() ==
							tissuestack::services::TissueStackTaskStatus::ERRONEOUS)))
		return true;

	return false;
}
