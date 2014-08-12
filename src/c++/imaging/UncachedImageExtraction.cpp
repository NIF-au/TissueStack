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
	//TODO: request obsolete check

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
	if (request->getContrastMinimum() != 0 && request->getContrastMaximum() != 255)
		this->changeContrast(
				img, imgInfo,
				request->getContrastMinimum(),
				request->getContrastMaximum(),
				image->getImageDataMinumum(),
				image->getImageDataMaximum(),
				actualDimension->getWidth(),
				actualDimension->getHeight());

	// perform color mapping if requested
	if (request->getColorMapName().compare("gray") != 0 ||
		request->getColorMapName().compare("grey") != 0)
		this->applyColorMap(
				img, imgInfo,
				request->getColorMapName(),
				actualDimension->getWidth(),
				actualDimension->getHeight());

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
	const char dim = actualDimension->getName().at(0);

	img = ConstituteImage(
		actualDimension->getWidth(),
		actualDimension->getHeight(),
		(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? "I" : "RGB",
		CharPixel,
		data, &exception);

	// sanity check: was graphics magick able to create an image based on what we gave it?
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImageInfo(imgInfo);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}

	if (image->getFormat() == tissuestack::imaging::FORMAT::RAW) return img;

	// we have to do some eye watering stuff to deal with legacy
	tissuestack::logging::TissueStackLogger::instance()->debug("Actual Dimension: %c\n", dim);
	tissuestack::logging::TissueStackLogger::instance()->debug("Order [0/1/2]: %c/%c/%c\n", dim_order[0].at(0), dim_order[1].at(0), dim_order[2].at(0));
	Image * tmp = img;
	if ((image->getFormat() == tissuestack::imaging::FORMAT::MINC) && (
		(dim == 'x'
			&& dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'x') ||
		(dim == 'z'
			&& dim_order[0].at(0) == 'z' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'y') ||
		((dim == 'y' || dim == 'z')
			&& dim_order[0].at(0) == 'x' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'y') ||
		(dim_order[0].at(0) == 'x' || dim_order[1].at(0) == 'y' || dim_order[2].at(0) == 'z') ||
		((dim == 'x' || dim == 'y')
			&& dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'z')))
	{
		if (dim_order[0].at(0) == 'x'
				&& dim_order[1].at(0) == 'z'
						&& dim_order[2].at(0) == 'y' &&
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
					const unsigned short minimum,
					const unsigned short maximum,
					const unsigned short dataset_min,
					const unsigned short dataset_max,
					const unsigned long int width,
					const unsigned long int height) const
{
	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	PixelPacket * pixels = GetImagePixelsEx(img, 0, 0, width, height, &exception);

	if (pixels == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);

		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Contrast Application: Could not obtain pixels from image!");
	}

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned long long int pixel_value;


	while (i < height)
	{
		j = 0;
		while (j < width)
		{
			pixel_value = static_cast<unsigned long long int>(pixels[(width * i) + j].red);
			if (QuantumDepth != 8 && img->depth == QuantumDepth) pixel_value =
					this->mapUnsignedValue(img->depth, 8, pixel_value);

			//pixel_value = get_contrasted_value(min, max, dataset_min, dataset_max, pixel_value);
			float contrast_min = static_cast<float>(minimum);
			float contrast_max = static_cast<float>(maximum);
			float val = static_cast<float>(pixel_value);

			if (val <= contrast_min)
				pixel_value = dataset_min;
			else if (val >= contrast_max)
				pixel_value = dataset_max;
			else
				pixel_value =
					static_cast<unsigned long long int>(
						lround(((val - contrast_min) / (contrast_max - contrast_min))
							* static_cast<float>(dataset_max - dataset_min)));

			// graphicmagick quantum depth mess which we have to react to at runtime
			pixels[(width * i) + j].red =
				pixels[(width * i) + j].green =
					pixels[(width * i) + j].blue = static_cast<unsigned char>(pixel_value);

			if (QuantumDepth == 16 && img->depth == QuantumDepth)
			{
				pixels[(width * i) + j].red =
					pixels[(width * i) + j].green =
						pixels[(width * i) + j].blue =
							static_cast<unsigned short>(this->mapUnsignedValue(8, 16, pixel_value));
			} else if (QuantumDepth == 32 && img->depth == QuantumDepth)
			{
				pixels[(width * i) + j].red =
					pixels[(width * i) + j].green =
						pixels[(width * i) + j].blue =
							static_cast<unsigned int>(this->mapUnsignedValue(8, 32, pixel_value));
			}
			j++;
		}
		i++;
	}
}

inline unsigned long long tissuestack::imaging::UncachedImageExtraction::mapUnsignedValue(const unsigned char fromBitRange, const unsigned char toBitRange, const unsigned long long value) const {
	// cap at 64 bits
	if (fromBitRange > 64 || toBitRange > 64) return 0;

	unsigned long long from = (static_cast<unsigned long long int>(1) << fromBitRange) - static_cast<unsigned long long int>(1);
	unsigned long long to = (static_cast<unsigned long long int>(1) << toBitRange) - static_cast<unsigned long long int>(1);

	// check if value exceeds its native range
	if (value > from) return 0;

	return static_cast<unsigned long long>(llround((static_cast<double>(value) / from) * static_cast<double>(to)));
}

void inline tissuestack::imaging::UncachedImageExtraction::applyColorMap(
		Image * img,
		ImageInfo * imgInfo,
		const std::string color_map_name,
		const unsigned long int width,
		const unsigned long int height) const
{
	// retrieve color map
	const tissuestack::imaging::TissueStackColorMap * colorMap =
			tissuestack::imaging::TissueStackColorMapStore::instance()->findColorMap(color_map_name);
	if (colorMap == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Colormap Application: Could not find color map!");

	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	PixelPacket * pixels = GetImagePixelsEx(img, 0, 0, width, height, &exception);

	if (pixels == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		DestroyImageInfo(imgInfo);

		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Colormap Application: Could not obtain pixels from image!");
	}

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned long long int pixel_value;

	while (i < height)
	{
		j = 0;
		while (j < width)
		{
			pixel_value = static_cast<unsigned long long int>(pixels[(width * i) + j].red);
			const std::array<const unsigned short, 3> mapping = colorMap->getRGBMapForGrayValue(pixel_value);
			if (QuantumDepth != 8 && img->depth == QuantumDepth) pixel_value =
					this->mapUnsignedValue(img->depth, 8, pixel_value);

			pixels[(width * i) + j].red = static_cast<unsigned char>(mapping[0]);
			pixels[(width * i) + j].green = static_cast<unsigned char>(mapping[1]);
			pixels[(width * i) + j].blue = static_cast<unsigned char>(mapping[2]);

			// graphicmagick quantum depth mess which we have to react to at runtime
			if (QuantumDepth == 16 && img->depth == QuantumDepth) {
				pixels[(width * i) + j].red =
					static_cast<unsigned short>(
						this->mapUnsignedValue(
							8, 16, static_cast<unsigned long long int>(mapping[0])));
				pixels[(width * i) + j].green =
					static_cast<unsigned short>(
						this->mapUnsignedValue(
							8, 16, static_cast<unsigned long long int>(mapping[1])));
				pixels[(width * i) + j].blue =
					static_cast<unsigned short>(
						this->mapUnsignedValue(
							8, 16, static_cast<unsigned long long int>(mapping[2])));
			} else if (QuantumDepth == 32 && img->depth == QuantumDepth) {
				pixels[(width * i) + j].red =
					static_cast<unsigned int>(
						this->mapUnsignedValue(
							8, 32, static_cast<unsigned long long int>(mapping[0])));
				pixels[(width * i) + j].green =
					static_cast<unsigned int>(
						this->mapUnsignedValue(
							8, 32, static_cast<unsigned long long int>(mapping[1])));
				pixels[(width * i) + j].blue =
					static_cast<unsigned int>(
						this->mapUnsignedValue(
							8, 32, static_cast<unsigned long long int>(mapping[2])));
			}
			j++;
		}
		i++;
	}
}
