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
		if (processing_strategy == nullptr || converter_task == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"PreTiling requires an instance of a processing strategy and a conversion task!");

		tissuestack::logging::TissueStackLogger::instance()->info(
			"Staring Conversion: %s => %s",
			converter_task->getInputImageData()->getFileName().c_str(),
			converter_task->getOutFile().c_str());



		// TODO: implement conversion
		// regularly check for shutdown/task status (cancellation)
		// make 'visitor' methods for minc and nifti, e.g convert(MincImageData), convert(NiftiImageData)
		const bool finished =
			const_cast<tissuestack::services::TissueStackConversionTask *>(converter_task)->incrementSlicesDone();
		tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(converter_task->getId());





		tissuestack::logging::TissueStackLogger::instance()->info(
			"Finished Conversion: %s => %s",
			converter_task->getInputImageData()->getFileName().c_str(),
			converter_task->getOutFile().c_str());

		// we take care of things in the online/server version
		if (finished && processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_converter_task.release()->getId());
	} catch (const std::exception & bad)
	{
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
