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
		if ((!processing_strategy->isOnlineStrategy() && dimension.empty()) || // offline (not multi processed)
				(processing_strategy->isOnlineStrategy() && converter_task->getSlicesDone() == 0)) // online (not resume)
			this->_file_descriptor = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		else
			this->_file_descriptor = open(outFile.c_str(), O_WRONLY);

		if (this->_file_descriptor <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not open out file for RAW conversion!");

		// write header if: online (not resumed), offline (not multi processed) or offline (multi processed + write header flag)
		if ((!processing_strategy->isOnlineStrategy() && (dimension.empty() || (!dimension.empty() && writeHeader))) ||
				(processing_strategy->isOnlineStrategy() && converter_task->getSlicesDone() == 0))
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

		// in the case of an offline multi-process we have to fast-forward to the desired offset
		if (!processing_strategy->isOnlineStrategy() && !dimension.empty())
		{
			lseek(this->_file_descriptor,
				converter_task->getInputImageData()->getDimensionByLongName(dimension)->getOffset(),
				SEEK_SET);
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
				"Starting/Resuming Conversion: %s => %s",
				fileName.c_str(),
				outFile.c_str());
		else if (dimension.empty())
			std::cout << "Starting Conversion: " << fileName << " => " << outFile << std::endl;

		// the dimension loop
		this->loopOverDimensions(processing_strategy, converter_task, dimension);

		if (this->hasBeenCancelledOrShutDown(processing_strategy, converter_task))
		{
			if (processing_strategy->isOnlineStrategy())
				ptr_converter_task.release();
			else
			{
				const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->stop();
				close(this->_file_descriptor);
				// we erase our partial work !
				unlink(converter_task->getOutFile().c_str());
			}

			return;
		}

		if (processing_strategy->isOnlineStrategy())
		{
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_converter_task.release()->getId());

			tissuestack::logging::TissueStackLogger::instance()->info(
				"Finished Conversion: %s => %s",
				fileName.c_str(),
				outFile.c_str());
		}
		else if (dimension.empty())
			std::cout << "Finished Conversion: " << fileName << " => " << outFile << std::endl;
	} catch (const std::exception & bad)
	{
		close(this->_file_descriptor);
		// we erase our partial work !
		unlink(converter_task->getOutFile().c_str());

		// flag task as error
		if (processing_strategy->isOnlineStrategy())
		{
			tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to convert: %s => %s: %s",
				converter_task->getInputImageData()->getFileName().c_str(),
				converter_task->getOutFile().c_str(), bad.what());

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

	mihandle_t volume = NULL;
	if (converter_task->getInputImageData()->getFormat() == tissuestack::imaging::FORMAT::MINC)
	{
		int result =
			miopen_volume(
				converter_task->getInputImageData()->getFileName().c_str(),
				MI2_OPEN_READ,
				&volume);
		if (result != MI_NOERROR)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"Failed to open supposed MINC file!");
	}

	// TODO: for dicom distinguish beetween 3 scenarios:
	// 1. single 2D image
	// 2. time series: 3 dims with third being time (2 coords/steps)
	// 3. true 3D data: address problem of gaps!
	if (converter_task->getInputImageData()->getFormat() == tissuestack::imaging::FORMAT::DICOM)
	{
		const tissuestack::imaging::TissueStackDicomData * dicomData =
			static_cast<const tissuestack::imaging::TissueStackDicomData *>(converter_task->getInputImageData());

		if (dicomData->getType() == tissuestack::imaging::DICOM_TYPE::SINGLE_IMAGE)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"DICOM SINGLE IMAGE NOT IMPLEMENTED YET!");
		}
		else if (dicomData->getType() == tissuestack::imaging::DICOM_TYPE::TIME_SERIES)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"DICOM TIME SERIES NOT IMPLEMENTED YET!");
		} else if (dicomData->getType() == tissuestack::imaging::DICOM_TYPE::VOLUME)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"DICOM VOLUME NOT IMPLEMENTED YET!");
	}

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
			order++;
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
				// fast forward file position
				unsigned long long int toBeContinuedAt =
					dim->getOffset() + sliceNumber * dim->getSliceSize() * 3;
				lseek(this->_file_descriptor, toBeContinuedAt, SEEK_SET);
				resumed = false;
			}

			// delegate to format specific conversion per slice
			if (converter_task->getInputImageData()->getFormat() == tissuestack::imaging::FORMAT::MINC)
				this->convertSlice(
					static_cast<const tissuestack::imaging::TissueStackMincData *>(
						converter_task->getInputImageData()),
						volume,
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

			//if (converter_task->getInputImageData()->getNumberOfDimensions() == 2)
			//	const_cast<tissuestack::services::TissueStackConversionTask *>(converter_task)->setSlicesDone(
			//		converter_task->getTotalSlices()-1);

			// shutdown/cancellation check
			if (this->hasBeenCancelledOrShutDown(processing_strategy, converter_task))
			{
				if (volume) miclose_volume(volume);
				return;
			}

			// increment slice progress
			const bool finished =
				const_cast<tissuestack::services::TissueStackConversionTask *>(converter_task)->incrementSlicesDone();

			// persist for the online tiling
			if (processing_strategy->isOnlineStrategy())
				tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(
					converter_task->getId());
			else if (!processing_strategy->isOnlineStrategy() && dimension.empty())
			{
				const std::string output =
					std::string("Progress:\t") +
					std::to_string(converter_task->getSlicesDone()) + "\t[" +
					std::to_string(converter_task->getTotalSlices()) + "]\t => " +
					std::to_string(converter_task->getProgress()) + "%\r";
				std::cout << output << std::flush;
			}
			else if (!processing_strategy->isOnlineStrategy() && !dimension.empty())
			{
				std::string output = "";
				for (unsigned short pos=0;pos<order;pos++)
					output += "\t\t\t";
				output += (dim->getName() + ": ");
				output += std::to_string(converter_task->getSlicesDone());
				output += (" [" + std::to_string(dim->getNumberOfSlices()) + "]");
				std::cout << output << "\r" << std::flush;
			}

			if (finished)
				return;
		}
		order++;
	}
	if (volume) miclose_volume(volume);
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
	{
		close(this->_file_descriptor);
		// we erase our partial work !
		unlink(converter_task->getOutFile().c_str());

		return true;
	}

	return false;
}


inline void tissuestack::imaging::RawConverter::convertSlice(
	const tissuestack::imaging::TissueStackMincData * minc,
	const mihandle_t & minc_handle,
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
			if (i != dimension_number && minc->getDimensionByOrderIndex(i))
				counts[i] = minc->getDimensionByOrderIndex(i)->getNumberOfSlices();
			else
				counts[i] = 1;
		}
		else
		{
			starts[i] = 0;
			counts[i] = 1;
		}
	}



	unsigned short rgbTotal = (minc->isColor() ? 3 : 1);
	unsigned char * buffer = new unsigned char[slice_size];

	unsigned long long int new_image_slice_size = dim->getSliceSize() * 3;
	unsigned char * newImage = new unsigned char[new_image_slice_size];

	for (unsigned short rgbChannel = 0; rgbChannel < rgbTotal; rgbChannel++)
	{
		if (rgbChannel > 0)
			starts[numOfDims-1] = rgbChannel;

		// read hyperslab as unsigned byte and let minc do the dirty deeds of conversion
		int result =
			miget_real_value_hyperslab(
				minc_handle,
				MI_TYPE_UBYTE,
				starts,
				counts,
				buffer);
		if (result != MI_NOERROR)
		{
			delete [] buffer;
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Minc => Raw Conversion failed: Could not read hyperslab!");
		}

		for (unsigned long long int i = 0; i < slice_size; i++)
		{
			if (!minc->isColor())
				newImage[i * 3 + 0] =
					newImage[i * 3 + 1] =
						newImage[i * 3 + 2] = buffer[i];
			else
				newImage[i * 3 + rgbChannel] = buffer[i];
		}
	}
	delete [] buffer;

	this->reorientMincSlice(minc, dim, newImage, 0);

	// write out into new raw file
	ssize_t bytesWritten =
		write(this->_file_descriptor, newImage, new_image_slice_size);
	delete [] newImage;
	if (bytesWritten != static_cast<ssize_t>(new_image_slice_size))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Minc Conversion: written bytes do not match expected bytes!");

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
	if (nifti->getNiftiHandle()->ndim > 3 ||
			(nifti->getNiftiHandle()->ndim == 3 && nifti->isColor())) dims[4] = 0;
	if (nifti->getNumberOfDimensions() == 2)
	{
		dims[dimension_number+1] = -1;
		dims[dimension_number+2] = -1;
	} else
		dims[dimension_number+1] = slice_number;

	const unsigned long long int size_per_slice =
		dim->getSliceSize();
	unsigned long long int expected_bytes =
		size_per_slice * static_cast<unsigned long long int>(nifti->getNiftiHandle()->nbyper);
	const unsigned long long int new_size_per_slice = size_per_slice * 3;

	unsigned short rgb_channel = 0;
	unsigned short rgb_total = (nifti->isColor() && nifti->getNiftiHandle()->ndim > 3) ? 3 : 1;
	unsigned char * data_out = new unsigned char[new_size_per_slice];

	while (rgb_channel < rgb_total)  // this loop is done once only for any data type other than RGB
	{
		if (nifti->isColor() && nifti->getNiftiHandle()->ndim > 3)
			dims[nifti->getNiftiHandle()->ndim] = rgb_channel;

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

inline void tissuestack::imaging::RawConverter::reorientMincSlice(
	const tissuestack::imaging::TissueStackMincData * minc,
	const tissuestack::imaging::TissueStackDataDimension * dim,
	unsigned char * data_out,
	const unsigned long int slice_number) const
{
	const std::vector<std::string> dimsOrder =
		minc->getDimensionOrder();

	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	Image * img = NULL;
	Image * tmp = NULL;

    if ((dimsOrder[0].at(0) == 'y' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'x' && dim->getName().at(0) == 'x') ||
        (dimsOrder[0].at(0) == 'z' && dimsOrder[1].at(0) == 'x' && dimsOrder[2].at(0) == 'y' && dim->getName().at(0) == 'z') ||
        (dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'y' && (dim->getName().at(0) == 'z' || dim->getName().at(0) == 'y')) ||
        (dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'y' && dimsOrder[2].at(0) == 'z') ||
        (dimsOrder[0].at(0) == 'y' && dimsOrder[1].at(0) == 'x' && dimsOrder[2].at(0) == 'z' && (dim->getName().at(0) == 'y' || dim->getName().at(0) == 'x')))
    {
    	img =
    		ConstituteImage(dim->getHeight(), dim->getWidth(), "RGB", CharPixel, data_out, &exception);

    	if (img == NULL)
    	{
    		delete [] data_out;
    		CatchException(&exception);
    		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
    			"Could not constitute Image!");
    	}

    	if ((dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'y' &&
    	        		(dim->getName().at(0) == 'z' || dim->getName().at(0) == 'y')))
    	{
			tmp = img;
			img = RotateImage(img, 90, &exception);
			DestroyImage(tmp);
			if (img == NULL)
			{
				delete [] data_out;
				CatchException(&exception);
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not rotate Image!");
			}
    	} else
    	{
			tmp = img;
			img = RotateImage(img, -90, &exception);
			if (img == NULL)
			{
				delete [] data_out;
				CatchException(&exception);
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not rotate Image!");
			}
    	}
    } else
    {
    	img = ConstituteImage(
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
    }

	if (!((dim->getName().at(0) == 'x' && dimsOrder[0].at(0) == 'y' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'x') ||
				(dim->getName().at(0) == 'z' && dimsOrder[0].at(0) == 'z' && dimsOrder[1].at(0) == 'x' && dimsOrder[2].at(0) == 'y') ||
				((dim->getName().at(0) == 'x' || dim->getName().at(0) == 'y') &&
					dimsOrder[0].at(0) == 'y' && dimsOrder[1].at(0) == 'x' && dimsOrder[2].at(0) == 'z') ||
				(dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'y' && dimsOrder[2].at(0) == 'z')))
	{
		tmp = img;
		img = FlipImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			delete [] data_out;
			CatchException(&exception);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Conversion: Failed to flip image!");
		}
	}

	/*
	if ((dimsOrder[0].at(0) == 'x' && dimsOrder[1].at(0) == 'z' && dimsOrder[2].at(0) == 'y')
		 && (dim->getName().at(0) == 'z' || dim->getName().at(0) == 'y'))
	{
		tmp = img;
		img = FlipImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			delete [] data_out;
			CatchException(&exception);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Conversion: Could not flip Image!");
		}
	}*/

	// extract pixel info, looping over values
	PixelPacket * pixels = GetImagePixels(img, 0, 0, dim->getWidth(), dim->getHeight());
	if (pixels == NULL)
	{
		delete [] data_out;
		if (img != NULL) DestroyImage(img);
		if (img == NULL)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"MINC conversion: Failed to get image pixels!");
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
