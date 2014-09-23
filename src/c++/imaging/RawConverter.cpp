#include "networking.h"
#include "imaging.h"
#include "services.h"

tissuestack::imaging::RawConverter::RawConverter(){}

void tissuestack::imaging::RawConverter::convert(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackConversionTask * converter_task,
	const std::string dimension,
	const bool writeHeader)
{
	// this will be useful for offline tools to not need to keep track of pointer
	std::unique_ptr<const tissuestack::services::TissueStackConversionTask> ptr_converter_task(converter_task);

	try
	{
		const std::string fileName = converter_task->getInputImageData()->getFileName();
		const std::string outFile = converter_task->getOutFile();

		// open our out file for writing and perform some basic checks
		if (dimension.empty())
			this->_file_descriptor = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			this->_file_descriptor = open(outFile.c_str(), O_WRONLY);

		if (this->_file_descriptor <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not open out file for RAW conversion!");

		// write header (if requested)
		if (dimension.empty() || (!dimension.empty() && writeHeader))
		{
			if (converter_task->getInputImageData()->getHeader().empty())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"We don't have a valid header for RAW conversion!");

			ssize_t bytesWritten =
				write(
					this->_file_descriptor,
					converter_task->getInputImageData()->getHeader().c_str(),
					converter_task->getInputImageData()->getHeader().length());
			if (bytesWritten != static_cast<ssize_t>(converter_task->getInputImageData()->getHeader().length()))
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not write header for RAW conversion!");
		}

		// shutdown/cancellation check
		if (this->hasBeenCancelledOrShutDown(processing_strategy, converter_task))
		{
			close(this->_file_descriptor);
			if (processing_strategy->isOnlineStrategy())
				ptr_converter_task.release();

			return;
		}

		// tell us that we are starting now
		if (processing_strategy->isOnlineStrategy())
			tissuestack::logging::TissueStackLogger::instance()->info(
				"Staring Conversion: %s => %s",
				fileName.c_str(),
				outFile.c_str());
		else
			std::cout << "Starting Conversion: " << fileName << " => " << outFile << std::endl;

		// the dimension loop
		this->loopOverDimensions(processing_strategy, converter_task, dimension);

		if (this->hasBeenCancelledOrShutDown(processing_strategy, converter_task))
		{
			if (processing_strategy->isOnlineStrategy())
				ptr_converter_task.release();
			else
				const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);

			return;
		}

		if (processing_strategy->isOnlineStrategy())
			tissuestack::logging::TissueStackLogger::instance()->info(
				"Finished Conversion: %s => %s",
				fileName.c_str(),
				outFile.c_str());
		else
			std::cout << "Finished Conversion: " << fileName << " => " << outFile << std::endl;
			const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);

	} catch (const std::exception & bad)
	{
		close(this->_file_descriptor);
		// we erase our partial work !
		unlink(converter_task->getOutFile().c_str());

		// flag task as error
		if (processing_strategy->isOnlineStrategy())
		{
			tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to convert: %s => %s",
				converter_task->getInputImageData()->getFileName().c_str(),
				converter_task->getOutFile().c_str());

			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsErroneous(
				ptr_converter_task.release()->getId());
		} else
		{
			std::cerr << "Error Conversion: " << bad.what() << std::endl;
			const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);
		}
	}
}

inline void tissuestack::imaging::RawConverter::loopOverDimensions(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::services::TissueStackConversionTask * converter_task,
		const std::string & dimension) const
{
	const std::vector<std::string> dimensionsToBeConverted =
			converter_task->getInputImageData()->getDimensionOrder();

	// this is for the resume of the online conversion
	bool resumed = false;
	if (processing_strategy->isOnlineStrategy() && converter_task->getSlicesDone() != 0)
		resumed = true;
	unsigned long long int dimOffset = 0;
	unsigned long long int accSliceNumber = 0;

	unsigned short order = 0;
	for (auto d : dimensionsToBeConverted) // the dimension loop
	{
		const tissuestack::imaging::TissueStackDataDimension * dim =
			converter_task->getInputImageData()->getDimensionByLongName(d);

		if (!processing_strategy->isOnlineStrategy() &&
			!dimension.empty() && dimension.compare(d) != 0)
		{
			order++;
			continue; // for offline version: we skip the dimensions not specified
		}

		accSliceNumber += dim->getNumberOfSlices();

		if (processing_strategy->isOnlineStrategy() && resumed &&
			converter_task->getSlicesDone() >= accSliceNumber)
		{
			dimOffset = accSliceNumber;
			continue;
		}

		unsigned long long int accNumber = dimOffset;
		for (unsigned long long int sliceNumber = 0; sliceNumber < dim->getNumberOfSlices(); sliceNumber++) // the slice loop
		{
			// resume at a previously interrupted point
			if (resumed && processing_strategy->isOnlineStrategy())
			{
				if (converter_task->getSlicesDone() >= accNumber)
				{
					++accNumber;
					continue;
				}
				resumed = false;
			}

			// delegate to format specific conversion per slice
			if (converter_task->getInputImageData()->getFormat() == tissuestack::imaging::FORMAT::MINC)
				this->convertSlice(
					static_cast<const tissuestack::imaging::TissueStackMincData *>(
						converter_task->getInputImageData()),
						sliceNumber,
						order);
			else if (converter_task->getInputImageData()->getFormat() == tissuestack::imaging::FORMAT::NIFTI)
				this->convertSlice(
					static_cast<const tissuestack::imaging::TissueStackNiftiData *>(
						converter_task->getInputImageData()),
						sliceNumber,
						order);
			else
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Image Format is not suitable for RAW conversion!");

			// shutdown/cancellation check
			if (this->hasBeenCancelledOrShutDown(processing_strategy, converter_task))
				return;

			// increment slice progress
			const bool finished =
				const_cast<tissuestack::services::TissueStackConversionTask *>(converter_task)->incrementSlicesDone();

			// persist for the online tiling
			if (processing_strategy->isOnlineStrategy())
				tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(
					converter_task->getId());
			else
			{
				const std::string output =
					std::string("Progress:\t") +
					std::to_string(converter_task->getSlicesDone()) + "\t[" +
					std::to_string(converter_task->getTotalSlices()) + "]\t => " +
					std::to_string(converter_task->getProgress()) + "%\r";

				std::string newLines = "";
				if (!dimension.empty())
				{
					for (unsigned short i=0;i<order;i++)
						newLines += "\n";
				}
				std::cout << newLines << output;
			}

			if (finished)
				return;
		}
		order++;
	}
}

inline const bool tissuestack::imaging::RawConverter::hasBeenCancelledOrShutDown(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackConversionTask * converter_task) const
{
	if (processing_strategy == nullptr)
		return true;

	// abortion check
	if (!processing_strategy->isRunning()
			|| processing_strategy->isStopFlagRaised())
		return true;

	// cancel check
	if (converter_task == nullptr
		||	(converter_task &&
				(converter_task->getStatus() ==
						tissuestack::services::TissueStackTaskStatus::CANCELLED
					|| converter_task->getStatus() ==
							tissuestack::services::TissueStackTaskStatus::ERRONEOUS)))
		return true;

	return false;
}


inline void tissuestack::imaging::RawConverter::convertSlice(
	const tissuestack::imaging::TissueStackMincData * minc,
	const unsigned long int slice_number,
	const short dimension_number) const
{
	unsigned short numOfDims =
		minc->isColor() ? minc->getNumberOfDimensions() + 1 :
			minc->getNumberOfDimensions();
	unsigned long starts[numOfDims];
	unsigned long counts[numOfDims];

	const tissuestack::imaging::TissueStackDataDimension * dim =
		minc->getDimensionByOrderIndex(dimension_number);
	if (dim == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Minc => Raw Conversion failed: Can not read a slice size of 0!");

	unsigned long long int slice_size = dim->getSliceSize();

	// initialize
	for (unsigned short i=0;i<numOfDims;i++)
	{
		starts[i] = (i == dimension_number) ? slice_number : 0;
		if (i<3)
		{
			counts[i] = 1;
			if (i != dimension_number && minc->getDimensionByOrderIndex(i))
				counts[i] = minc->getDimensionByOrderIndex(i)->getNumberOfSlices();
		}
		else counts[i] = 1;
		std::cout << "start[ " << std::to_string(i) << "] => " << std::to_string(starts[i]) << std::endl;
		std::cout << "counts[ " << std::to_string(i) << "] => " << std::to_string(counts[i]) << std::endl;
	}
	std::cout << "Slice Size: " << std::to_string(slice_size) << std::endl;

	void * buffer = malloc(static_cast<size_t>(slice_size * minc->getMincTypeSize()));
	//memset(buffer, 0, slice_size);

	// read hyperslab as unsigned byte and let minc do the dirty deeds of conversion
	int result =
			/*
		miget_voxel_value_hyperslab(
			minc->getMincHandle(),
			minc->getMincType(),
			starts,
			counts,
			buffer);
	*/
		miget_real_value_hyperslab(
			minc->getMincHandle(),
			MI_TYPE_UBYTE,
			starts,
			counts,
			buffer);
	if (result != MI_NOERROR)
	{
		std::cout << std::to_string(result) << std::endl;
		free(buffer);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Minc => Raw Conversion failed: Could not read hyperslab!");
	}

	unsigned long long int new_image_slice_size = dim->getSliceSize() * 3;
	unsigned char * newImage = new unsigned char[new_image_slice_size];

	for (unsigned long long int i = 0; i < slice_size; i++)
	{
		newImage[i * 3 + 0] =
			newImage[i * 3 + 1] =
				newImage[i * 3 + 2] = static_cast<unsigned short *>(buffer)[i];

		//if (rgb_channel < 0) out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = val;
		//else out[i * 3 + rgb_channel] = val;
	}
	free(buffer);

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	Image * img = ConstituteImage(
		dim->getWidth(),
		dim->getHeight(),
		"RGB", CharPixel,
		newImage, &exception);
	delete [] newImage;
	// sanity check: was graphics magick able to create an image based on what we gave it?
	if (img == NULL)
	{
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}
	ImageInfo * imgInfo = CloneImageInfo((ImageInfo *)NULL);

	const std::string fileName =
		std::string("/tmp/") +
		dim->getName().substr(0, 1) +
		"_" + std::to_string(slice_number) + ".png";
	strcpy(img->filename, fileName.c_str());

	if (WriteImage(imgInfo, img) == MagickFail)
	{
		CatchException(&img->exception);
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to write out image: %s\n", img->exception.reason);
	}

	// tidy up
	if (img) DestroyImage(img);
	if (imgInfo) DestroyImageInfo(imgInfo);

}

inline void tissuestack::imaging::RawConverter::convertSlice(
	const tissuestack::imaging::TissueStackNiftiData * nifti,
	const unsigned long int slice_number,
	const short dimension_number) const
{
}
