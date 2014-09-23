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
			{
				const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);
				close(this->_file_descriptor);
				// we erase our partial work !
				unlink(converter_task->getOutFile().c_str());
			}

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
				std::cout << newLines << output << std::flush;
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
	int dims[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };
	void *data_in = NULL;

	const tissuestack::imaging::TissueStackDataDimension * dim =
			nifti->getDimensionByOrderIndex(dimension_number);
	if (dim == nullptr)
		THROW_TS_EXCEPTION(
			tissuestack::common::TissueStackApplicationException,
			"Could not find dimension of NIFTI file to read data!");

	// set time slice to 0 for any data set with dimensionality greater than 3
	if (nifti->getNiftiHandle()->ndim > 3) dims[4] = 0;
	dims[dimension_number+1] = slice_number;

	const unsigned long long int size_per_slice =
		dim->getSliceSize();
	unsigned long long int expected_bytes =
		size_per_slice * static_cast<unsigned long long int>(nifti->getNiftiHandle()->nbyper);
	const unsigned long long int new_size_per_slice = size_per_slice * 3;

	unsigned short rgb_channel = 0;
	unsigned short rgb_total = (nifti->isColor()) ? 3 : 1;
	unsigned char * data_out = new unsigned char[new_size_per_slice];

	while (rgb_channel < rgb_total)  // this loop is done once only for any data type other than RGB
	{
		if (nifti->isColor()) dims[nifti->getNiftiHandle()->ndim] = rgb_channel;

		int ret = nifti_read_collapsed_image(
				const_cast<nifti_image *>(nifti->getNiftiHandle()), dims, &data_in);
		if (ret < 0)
		{
			delete [] data_out;
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"Failed to read NIFTI file!");
		}

		//sanity check
		if (static_cast<unsigned long long int>(ret) != expected_bytes)
		{
			if (data_in)
			{
				free(data_in);
				data_in = NULL;
			}
			delete [] data_out;
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"NIFTI read: number of read bytes does not match expected bytes!");
		}

		// loop over pixels
		this->iteratOverPixelsAndConvert(
			data_in,
			data_out,
			size_per_slice,
			nifti->getNiftiHandle(),
			nifti->getMin(),
			nifti->getMax(),
			nifti->isColor(),
			rgb_channel);
		if (data_in)
		{
			free(data_in);
			data_in = NULL;
		}
		if (data_out == NULL)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"Failed to convert NIFTI file!");
		rgb_channel++;
	}

	this->reorientNiftiSlice(nifti, dim, data_out, 0);

	// write out into new raw file
	ssize_t bytesWritten =
		write(this->_file_descriptor, data_out, new_size_per_slice);
	delete [] data_out;
	if (bytesWritten != static_cast<ssize_t>(new_size_per_slice))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Nifti Conversion: written bytes do not match expected bytes!");
}

inline void tissuestack::imaging::RawConverter::iteratOverPixelsAndConvert(
	void * in,
	unsigned char * out,
	const unsigned long long int size,
	const nifti_image * nifti,
	const double min,
	const double max,
	const bool isRgb,
	const unsigned short rgb_channel) const {
	unsigned short error = 0;

	for (unsigned int i = 0; i < size; i++) {
		// keep track of error
		error = 0;
		// move start back "data type" number of bytes...
		if (i != 0)
			in = (void *) (((unsigned char*) in) + nifti->nbyper);
		// range to map to
		long double range = max - min;
		unsigned char val = 0;

		// now extract value
		switch (nifti->datatype) {
			case NIFTI_TYPE_UINT8: // unsigned char | nothing much to there but copy values
				val = (unsigned char) ((unsigned char *) in)[0];
				break;
			case NIFTI_TYPE_INT8: // signed char | adjust range to min/max found previously
				val = (unsigned char)
						roundl(((((long double) ((char *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT16: // unsigned short
				val = (unsigned char)
						roundl(((((long double) ((unsigned short *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT32: // unsigned int
				val = (unsigned char)
						roundl(((((long double) ((unsigned int *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT16: // signed short
				val = (unsigned char)
						roundl(((((long double) ((short *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT32: // signed int
				val = (unsigned char)
						roundl(((((long double) ((int *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_UINT64: // unsigned long long
				val = (unsigned char)
						roundl(((((long double) ((unsigned long long int *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_INT64: // signed long long
				val = (unsigned char)
						roundl(((((long double) ((long long int *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT32: //	float
				val = (unsigned char)
						roundl(((((long double) ((float *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT64: //	double
				val = (unsigned char)
						roundl(((((long double) ((double *) in)[0]) - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_FLOAT128: // long double
				val = (unsigned char)
						roundl(((((long double *) in)[0] - min) / range) * (long double) 255);
				break;
			case NIFTI_TYPE_RGB24:
			case NIFTI_TYPE_RGBA32: // unsigned char per channel
				val = (unsigned char) ((unsigned char *) in)[0];
				out[i * 3 + 0] = val;
				out[i * 3 + 1] = (unsigned char) ((unsigned char *) in)[1];
				out[i * 3 + 2] = (unsigned char) ((unsigned char *) in)[2];
				break;
			case 0:				// UNKNOWN
				error = 1;
				break;
			case DT_BINARY:				// NOT SUPPORTED
			case NIFTI_TYPE_COMPLEX64:
			case NIFTI_TYPE_COMPLEX128:
			case NIFTI_TYPE_COMPLEX256:
				error = 1;
				break;
			default:	//	even more unknown
				error = 1;
				break;
			}
		// set value in out array, depending on whether we have rgb or not
		if (!isRgb)
		{
			if (nifti->datatype != NIFTI_TYPE_RGB24 && nifti->datatype != NIFTI_TYPE_RGBA32)
				out[i * 3 + 0] = out[i * 3 + 1] = out[i * 3 + 2] = val;
		} else out[i * 3 + rgb_channel] = val;

		// check for error
		if (error)
		{
			// free and good bye
			if (in)
			{
				free(in);
				in = NULL;
			}
			delete [] out;
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"Failed to convert NIFTI file: Unknown or unsupported data type!");

		}
	}
}

inline void tissuestack::imaging::RawConverter::reorientNiftiSlice(
	const tissuestack::imaging::TissueStackNiftiData * nifti,
	const tissuestack::imaging::TissueStackDataDimension * dim,
	unsigned char * data_out,
	const unsigned long int slice_number) const
{
	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	Image * img = ConstituteImage(
		dim->getWidth(),
		dim->getHeight(),
		"RGB", CharPixel,
		data_out, &exception);

	if (img == NULL)
	{
		delete [] data_out;
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}

	Image * tmp = img;
	img = FlipImage(img, &exception);
	DestroyImage(tmp);
	if (img == NULL)
	{
		delete [] data_out;
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not flip Image!");
	}

	const std::vector<std::string> dimsOrder =
		nifti->getDimensionOrder();

	if ((dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'y')
		 && (dim->getName().at(0) == 'z' || dim->getName().at(0) == 'y'))
	{
		tmp = img;
		img = FlopImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			delete [] data_out;
			CatchException(&exception);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not flop Image!");
		}
	}

	// extract pixel info, looping over values
	PixelPacket * pixels = GetImagePixels(img, 0, 0, dim->getWidth(), dim->getHeight());
	if (pixels == NULL)
	{
		delete [] data_out;
		if (img != NULL) DestroyImage(img);
		if (img == NULL)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"NIFTI conversion: Failed to get image pixels!");
	}

	// sync with data
	for (unsigned long long int j = 0; j < dim->getSliceSize(); j++)
	{
		// graphicsmagic quantum depth correction
		if (QuantumDepth != 8 && img->depth == QuantumDepth)
		{
			data_out[j * 3 + 0] =
				static_cast<unsigned char>(
					this->mapUnsignedValue(img->depth, 8, pixels[j].red));
			data_out[j * 3 + 1] =
				static_cast<unsigned char>(
					this->mapUnsignedValue(img->depth, 8, pixels[j].green));
			data_out[j * 3 + 2] =
				static_cast<unsigned char>(
					this->mapUnsignedValue(img->depth, 8, pixels[j].blue));
			continue;
		} // no correction needed
		data_out[j * 3 + 0] = (unsigned char) pixels[j].red;
		data_out[j * 3 + 1] = (unsigned char) pixels[j].green;
		data_out[j * 3 + 2] = (unsigned char) pixels[j].blue;
	}

	ImageInfo * imgInfo = NULL;
	if (slice_number != 0)
	{
		imgInfo = CloneImageInfo((ImageInfo *)NULL);
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
	}

	// tidy up
	if (img) DestroyImage(img);
	if (imgInfo) DestroyImageInfo(imgInfo);
}

inline unsigned long long tissuestack::imaging::RawConverter::mapUnsignedValue(
		const unsigned char fromBitRange, const unsigned char toBitRange, const unsigned long long value) const {
	// cap at 64 bits
	if (fromBitRange > 64 || toBitRange > 64) return 0;

	unsigned long long from = (static_cast<unsigned long long int>(1) << fromBitRange) - static_cast<unsigned long long int>(1);
	unsigned long long to = (static_cast<unsigned long long int>(1) << toBitRange) - static_cast<unsigned long long int>(1);

	// check if value exceeds its native range
	if (value > from) return 0;

	return static_cast<unsigned long long>(llround((static_cast<double>(value) / from) * static_cast<double>(to)));
}
