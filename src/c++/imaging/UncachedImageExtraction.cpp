#include "imaging.h"

tissuestack::imaging::UncachedImageExtraction::UncachedImageExtraction()
{

}

void tissuestack::imaging::UncachedImageExtraction::extractImage(
						const tissuestack::imaging::TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	// delegate
	this->extractImage(-1, image, request, slice);
}

void tissuestack::imaging::UncachedImageExtraction::extractImage(
						const int descriptor,
						const tissuestack::imaging::TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	// determine some parameters for data reading
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	long long int dataLength =
			actualDimension->getSliceSize() *
			static_cast<long long int>(
					(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? 1 : 3);
	unsigned long long int actualOffset =
			actualDimension->getOffset() +	request->getSliceNumber() * static_cast<unsigned long long int>(dataLength);

	ExceptionInfo exception;
	ImageInfo	*imgInfo = NULL;

	GetExceptionInfo(&exception);
	imgInfo = CloneImageInfo((ImageInfo *)NULL);
	if (imgInfo == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create ImageInfo!");

	// read the actual raw data to write out images later on
	std::unique_ptr<unsigned char[]> data(new unsigned char[dataLength]);
	memset(data.get(), '\0', dataLength);
	ssize_t bRead =
		pread64(
			const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor(),
			data.get(),
			dataLength,
			actualOffset);
	if (bRead != dataLength)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to read entire slice from RAW file!");

	// start graphics magick  image instantiation
	Image * img =
		this->createImageFromDataRead(
			image,
			actualDimension,
			data.get(),
			imgInfo);

	// apply possible contrast settings
	// TODO: append request params
	//if (request->getContrast() != 0)
	//	this->changeContrast(img, imgInfo, 0, 255);

	// perform color mapping if requested
	if (request->getColorMapName().compare("gray") != 0 ||
		request->getColorMapName().compare("grey") != 0)
		this->applyColorMap(img, imgInfo, request->getColorMapName());

	// adjust scale (if requested)
	if (request->getScaleFactor() != static_cast<const float>(1.0))
		img =
			this->scaleImage(
				img,
				imgInfo,
				static_cast<const float>(actualDimension->getWidth()) * request->getScaleFactor(),
				static_cast<const float>(actualDimension->getHeight()) * request->getScaleFactor());

	// adjust quality (if requested)
	if (request->getQualityFactor() < static_cast<const float>(1.0))
		img =
			this->degradeImage(
				img,
				imgInfo,
				static_cast<const float>(actualDimension->getWidth()) * request->getScaleFactor(),
				static_cast<const float>(actualDimension->getHeight()) * request->getScaleFactor(),
				request->getQualityFactor());

	// this is the part were we start to serialize the output of our finished image work
	std::string formatLowerCase =  request->getOutputImageFormat();
	std::transform(formatLowerCase.begin(), formatLowerCase.end(), formatLowerCase.begin(), tolower);
    strcpy(img->magick, formatLowerCase.c_str());
    std::string image_format("image/");

    // add the header beforehand
    const std::string httpResponseHeader =
    		 tissuestack::utils::Misc::composeHttpResponse(
    				 "200 OK",
    				 image_format + formatLowerCase,
    				 ""
    );

    FILE * handle = fdopen(descriptor, "wr");
    imgInfo->file = handle;
    write(descriptor, httpResponseHeader.c_str(), httpResponseHeader.length());
    WriteImage(imgInfo, img);

    // tidy up
    if (img) DestroyImage(img);
    if (imgInfo) DestroyImageInfo(imgInfo);
    fclose(handle);
}

inline Image * tissuestack::imaging::UncachedImageExtraction::createImageFromDataRead(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned char * data,
		ImageInfo * imgInfo) const
{
	Image * img = NULL;
	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	const std::vector<std::string> dim_order = image->getDimensionOrder();
	const char dim =	actualDimension->getName().at(0);

	if (image->getFormat() == tissuestack::imaging::FORMAT::MINC && (
		(dim == 'x'
			&& dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'x') ||
		(dim == 'y'
			&& dim_order[0].at(0) == 'x' && dim_order[1].at(0) == 'z'	&& dim_order[2].at(0) == 'y') ||
		(dim == 'z'
			&& dim_order[0].at(0) == 'z' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'y') ||
		(dim_order[0].at(0) == 'x' || dim_order[1].at(0) == 'y' || dim_order[2].at(0) == 'z') ||
		((dim == 'x' || dim == 'y')
			&& dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'z')))
	{
		img = ConstituteImage(
			actualDimension->getHeight(),
			actualDimension->getWidth(),
			(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? "I" : "RGB",
			CharPixel,
			data, &exception);
	} else
	{
		// this is the norm really!!
		img = ConstituteImage(
			actualDimension->getWidth(),
			actualDimension->getHeight(),
			(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? "I" : "RGB",
			CharPixel,
			data, &exception);
	}

	// sanity check: was graphics magick able to create an image based on what we gave it?
	if (img == NULL)
	{
	    CatchException(&exception);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not constitute Image!");
	}
	// we are good if we are within the newer RAW format
	if (image->getFormat() == tissuestack::imaging::FORMAT::RAW) return img;

	// we have to do some eye watering stuff to deal with legacy
	Image * tmp = img;
	if ((dim_order[0].at(0) == 'x'
			&& dim_order[1].at(0) == 'z'
					&& dim_order[2].at(0) == 'y') ||
			(dim == 'z' || dim == 'y'))
	{
		img = RotateImage(img, 90, &exception);
	} else img = RotateImage(img, -90, &exception);

	DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to rotate image to make it backward compatible!");
	}

	if (image->getFormat() == tissuestack::imaging::FORMAT::NIFTI ||
			(image->getFormat() == tissuestack::imaging::FORMAT::MINC &&
				!((dim == 'x' && dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'x') ||
				(dim == 'z' && dim_order[0].at(0) == 'z' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'y') ||
				((dim == 'x' || dim == 'y') && dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'z') ||
				(dim_order[0].at(0) == 'x' && dim_order[1].at(0) == 'y' && dim_order[2].at(0) == 'z'))))
	{
		tmp = img;
		img = FlipImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			DestroyImageInfo(imgInfo);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Image Extraction: Failed to flip image to make it backward compatible!");
		}
	}

	if ((dim == 'y' || dim == 'z') &&
			dim_order[0].at(0) == 'x' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'y')
	{
		tmp = img;
		img = FlipImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			DestroyImageInfo(imgInfo);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Image Extraction: Failed to flop image to make it backward compatible!");
		}
	}

	return img;
}
inline Image * tissuestack::imaging::UncachedImageExtraction::degradeImage(
		Image * img,
		ImageInfo * imgInfo,
		const unsigned int width,
		const unsigned int height,
		const float quality_factor) const
{
	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	Image * tmp = img;
	img =
		SampleImage(
			img,
			static_cast<const float>(width) * quality_factor,
			static_cast<const float>(height) * quality_factor,
			&exception);
	DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to adjust quality!");
	}
	tmp = img;
	img =
		SampleImage(
			img,
			static_cast<const float>(width),
			static_cast<const float>(height),
			&exception);
	DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to adjust quality!");
	}

	return img;
}

inline Image * tissuestack::imaging::UncachedImageExtraction::scaleImage(
		Image * img,
		ImageInfo * imgInfo,
		const unsigned int width,
		const unsigned int height) const
{
	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	Image * tmp = img;
	img =
		ScaleImage(
			img,
			width,
			height,
			&exception);
	DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to adjust quality!");
	}

	return img;
}

void inline tissuestack::imaging::UncachedImageExtraction::changeContrast(
					Image * img,
					ImageInfo * imgInfo,
					unsigned short minimum,
					unsigned short maximum) const
{
	//TODO implement
}

void inline tissuestack::imaging::UncachedImageExtraction::applyColorMap(
		Image * img,
		ImageInfo * imgInfo,
		const std::string color_map_name) const
{
	//TODO implement
}

