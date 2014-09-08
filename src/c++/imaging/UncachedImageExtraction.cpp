#include "networking.h"
#include "imaging.h"

tissuestack::imaging::UncachedImageExtraction::UncachedImageExtraction()
{

}

Image * tissuestack::imaging::UncachedImageExtraction::extractImageOnly(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	// determine some parameters for data reading
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	long long int dataLength =
			actualDimension->getSliceSize() *
			static_cast<long long int>(
					(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? 1 : 3);
	unsigned long long int actualOffset =
			actualDimension->getOffset() +
				static_cast<unsigned long long int>(request->getSliceNumber()) * static_cast<unsigned long long int>(dataLength);

	// read the actual raw data to write out images later on
	std::unique_ptr<unsigned char[]> data(new unsigned char[dataLength]);
	//memset(data.get(), '\0', dataLength);
	const int fd =
		const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor();
	lseek(
		fd,
		actualOffset,
		SEEK_SET);
	ssize_t bRead =
		read(
			fd,
			static_cast<void *>(data.get()),
			dataLength);
	/*
	ssize_t bRead =
		pread64(
			const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor(),
			data.get(),
			dataLength,
			actualOffset);
	*/
	if (bRead != dataLength)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to read entire slice from RAW file!");

	if (request->hasExpired())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");

	return
		this->createImageFromDataRead(
			image,
			actualDimension,
			data.get());
}

Image * tissuestack::imaging::UncachedImageExtraction::applyPostExtractionTasks(
		Image * img,
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	if (img == NULL) return NULL;

	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	// apply possible contrast settings
	if (!(request->getContrastMinimum() == 0 && request->getContrastMaximum() == 255))
	{
		img = this->convertAnythingToRgbImage(img);
		this->changeContrast(
				img,
				request->getContrastMinimum(),
				request->getContrastMaximum(),
				image->getImageDataMinumum(),
				image->getImageDataMaximum(),
				actualDimension->getWidth(),
				actualDimension->getHeight());
	}

	// perform color mapping if requested
	if (request->getColorMapName().compare("gray") != 0 &&
		request->getColorMapName().compare("grey") != 0)
	{
		img = this->convertAnythingToRgbImage(img);
		this->applyColorMap(
				img,
				request->getColorMapName(),
				actualDimension->getWidth(),
				actualDimension->getHeight());
	}

	// adjust scale (if requested)
	if (request->getScaleFactor() != static_cast<const float>(1.0))
		img =
			this->scaleImage(
				img,
				static_cast<const float>(actualDimension->getWidth()) * request->getScaleFactor(),
				static_cast<const float>(actualDimension->getHeight()) * request->getScaleFactor());

	// adjust quality (if requested)
	if (request->getQualityFactor() < static_cast<const float>(1.0))
		img =
			this->degradeImage(
				img,
				static_cast<const float>(actualDimension->getWidth()) * request->getScaleFactor(),
				static_cast<const float>(actualDimension->getHeight()) * request->getScaleFactor(),
				request->getQualityFactor());

	// we don't have a preview => chop up into tiles
	if (!request->isPreview())
		img = this->getImageTile(img, request);

	return img;
}

const std::array<unsigned long long int, 3> tissuestack::imaging::UncachedImageExtraction::performQuery(
						const tissuestack::imaging::TissueStackRawData * image,
						const tissuestack::networking::TissueStackQueryRequest * request) const
{
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	if (request->getXCoordinate() > actualDimension->getWidth() ||
			request->getYCoordinate() > actualDimension->getHeight())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image Query: Coordinate (x/y) exceeds the width/height of the image slice!");

	long long int dataLength =
			actualDimension->getSliceSize() *
			static_cast<long long int>(
					(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? 1 : 3);
	unsigned long long int actualOffset =
			actualDimension->getOffset() +
				static_cast<unsigned long long int>(request->getSliceNumber()) * static_cast<unsigned long long int>(dataLength);

	std::array<unsigned long long int, 3> pixel_value;
	if (image->getFormat() == tissuestack::imaging::FORMAT::RAW)
	{
		actualOffset +=
			static_cast<unsigned long long int>(
					static_cast<unsigned long long int>(request->getYCoordinate())*actualDimension->getWidth()*3 +
					static_cast<unsigned long long int>(request->getXCoordinate()*3));

		std::unique_ptr<unsigned char[]> data(new unsigned char[3] {'\0', '\0', '\0'});
		//memset(data.get(), '\0', 3);
		const int fd =
			const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor();
		lseek(
			fd,
			actualOffset,
			SEEK_SET);
		ssize_t bRead =
			read(
				fd,
				static_cast<void *>(data.get()),
				3);
		/*
		ssize_t bRead =
			pread64(
				const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor(),
				data.get(),
				3,
				actualOffset);
		*/
		if (bRead != 3)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Failed to query slice within RAW file!");

		pixel_value[0] = static_cast<unsigned long long int>(data[0]);
		pixel_value[1] = static_cast<unsigned long long int>(data[1]);
		pixel_value[2] = static_cast<unsigned long long int>(data[2]);

		return pixel_value;
	}

	Image * img =
		this->extractImageOnly(image, request);
	if (img == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create Image");

	PixelPacket pixels =
		GetOnePixel(
			img,
			request->getXCoordinate(),
			request->getYCoordinate());

	pixel_value[0] = static_cast<unsigned long long int>(pixels.red);
	pixel_value[1] = static_cast<unsigned long long int>(pixels.green);
	pixel_value[2] = static_cast<unsigned long long int>(pixels.blue);
	if (QuantumDepth != 8 && img->depth == QuantumDepth) {
		pixel_value[0] = this->mapUnsignedValue(img->depth, 8, pixel_value[0]);
		pixel_value[1] = this->mapUnsignedValue(img->depth, 8, pixel_value[1]);
		pixel_value[2] = this->mapUnsignedValue(img->depth, 8, pixel_value[2]);
	}
	DestroyImage(img);

	return pixel_value;
}

const Image * tissuestack::imaging::UncachedImageExtraction::extractImage(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::imaging::TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	Image * img =
		this->extractImageOnly(
			image,
			request);
	if (img == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create Image");


	// timeout/shutdown check
	if (request->hasExpired() || processing_strategy->isStopFlagRaised())
	{
		DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");
	}

	img =
		this->applyPostExtractionTasks(
			img,
			image,
			request);
	if (img == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not apply post extraction tasks to image");

	return img;
}

inline Image * tissuestack::imaging::UncachedImageExtraction::createImageFromDataRead(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned char * data) const
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}

	if (image->getFormat() == tissuestack::imaging::FORMAT::RAW) return img;

	/*
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
	}*/

	Image * tmp = img;
	if (image->getFormat() == tissuestack::imaging::FORMAT::NIFTI ||
			(image->getFormat() == tissuestack::imaging::FORMAT::MINC &&
				!((dim == 'x' && dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'z' && dim_order[2].at(0) == 'x') ||
				(dim == 'z' && dim_order[0].at(0) == 'z' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'y') ||
				((dim == 'x' || dim == 'y') && dim_order[0].at(0) == 'y' && dim_order[1].at(0) == 'x' && dim_order[2].at(0) == 'z') ||
				(dim_order[0].at(0) == 'x' && dim_order[1].at(0) == 'y' && dim_order[2].at(0) == 'z'))))
	{
		img = FlipImage(img, &exception);
		DestroyImage(tmp);
		if (img == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
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
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Image Extraction: Failed to flop image to make it backward compatible!");
		}
	}

	return img;
}

inline Image * tissuestack::imaging::UncachedImageExtraction::degradeImage(
		Image * img,
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to adjust quality!");
	}

	return img;
}

inline Image * tissuestack::imaging::UncachedImageExtraction::scaleImage(
		Image * img,
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to adjust quality!");
	}

	return img;
}

inline Image * tissuestack::imaging::UncachedImageExtraction::convertAnythingToRgbImage(Image * img) const
{
	if (img != NULL && img->is_grayscale)
	{
		ExceptionInfo exception;
		GetExceptionInfo(&exception);
		const PixelPacket * pixels = AcquireImagePixels(img, 0, 0, img->columns, img->rows, &exception);
		if (pixels == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"RGB conversion: Could not obtain pixels from image!");
		}

		ImageInfo * newImageInfo = CloneImageInfo((ImageInfo *)NULL);
		if (newImageInfo == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"RGB Conversion: Could not create new image info!");
		}
		newImageInfo->colorspace = ColorspaceType::RGBColorspace;

		Image * newRGBImage = AllocateImage(newImageInfo);
		if (newRGBImage == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			DestroyImageInfo(newImageInfo);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"RGB Conversion: Could not allocate new RGB image!");
		}
		newRGBImage->rows = img->rows;
		newRGBImage->columns = img->columns;

		// get pixels and copy over value
		PixelPacket * rgbPixels =  SetImagePixelsEx(newRGBImage, 0, 0, newRGBImage->columns, newRGBImage->rows, &exception);
		if (rgbPixels == NULL)
		{
			CatchException(&exception);
			DestroyImage(img);
			DestroyImage(newRGBImage);
			DestroyImageInfo(newImageInfo);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"RGB conversion: Could not obtain pixels from new RGB image!");
		}
		for (unsigned long int i=0;i<newRGBImage->columns*newRGBImage->rows;i++)
		{
			rgbPixels[i].red = pixels[i].red;
			rgbPixels[i].green = pixels[i].green;
			rgbPixels[i].blue = pixels[i].blue;
		}

		DestroyImage(img);
		DestroyImageInfo(newImageInfo);
		return newRGBImage;
	}

	return img;
}

void inline tissuestack::imaging::UncachedImageExtraction::changeContrast(
					Image * img,
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Contrast Application: Could not obtain pixels from image!");
	}

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned long long int pixel_values[3];

	float contrast_min = static_cast<float>(minimum);
	float contrast_max = static_cast<float>(maximum);

	while (i < height)
	{
		j = 0;
		while (j < width)
		{
			pixel_values[0] = static_cast<unsigned long long int>(pixels[(width * i) + j].red);
			pixel_values[1] = static_cast<unsigned long long int>(pixels[(width * i) + j].green);
			pixel_values[2] = static_cast<unsigned long long int>(pixels[(width * i) + j].blue);

			for (int z=0;z<3;z++)
			{
				if (QuantumDepth != 8 && img->depth == QuantumDepth)
					pixel_values[z] =
						this->mapUnsignedValue(img->depth, 8, pixel_values[z]);

				float val = static_cast<float>(pixel_values[z]);

				if (val <= contrast_min)
					pixel_values[z] = dataset_min;
				else if (val >= contrast_max)
					pixel_values[z] = dataset_max;
				else
					pixel_values[z] =
						static_cast<unsigned long long int>(
							lround(((val - contrast_min) / (contrast_max - contrast_min))
								* static_cast<float>(dataset_max - dataset_min)));
			}

			// graphicmagick quantum depth mess which we have to react to at runtime
			if (QuantumDepth == 16 && img->depth == QuantumDepth)
			{
				pixels[(width * i) + j].red = static_cast<unsigned short>(this->mapUnsignedValue(8, 16, pixel_values[0]));
				pixels[(width * i) + j].green = static_cast<unsigned short>(this->mapUnsignedValue(8, 16, pixel_values[1]));
				pixels[(width * i) + j].blue =	static_cast<unsigned short>(this->mapUnsignedValue(8, 16, pixel_values[2]));
			} else if (QuantumDepth == 32 && img->depth == QuantumDepth)
			{
				pixels[(width * i) + j].red = static_cast<unsigned int>(this->mapUnsignedValue(8, 32, pixel_values[0]));
				pixels[(width * i) + j].green = static_cast<unsigned int>(this->mapUnsignedValue(8, 32, pixel_values[1]));
				pixels[(width * i) + j].blue = static_cast<unsigned int>(this->mapUnsignedValue(8, 32, pixel_values[2]));
			} else
			{
				pixels[(width * i) + j].red = static_cast<unsigned char>(pixel_values[0]);
				pixels[(width * i) + j].green = static_cast<unsigned char>(pixel_values[1]);
				pixels[(width * i) + j].blue = static_cast<unsigned char>(pixel_values[2]);
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
		const std::string color_map_name,
		const unsigned long int width,
		const unsigned long int height) const
{
	// retrieve color map
	const tissuestack::imaging::TissueStackColorMap * colorMap =
			tissuestack::imaging::TissueStackColorMapStore::instance()->findColorMap(color_map_name);
	if (colorMap == nullptr)
	{
		DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Colormap Application: Could not find color map!");
	}

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	PixelPacket * pixels = GetImagePixelsEx(img, 0, 0, width, height, &exception);
	if (pixels == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
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
			if (QuantumDepth != 8 && img->depth == QuantumDepth) pixel_value =
							this->mapUnsignedValue(img->depth, 8, pixel_value);
			const std::array<const unsigned short, 3> mapping =
					colorMap->getRGBMapForGrayValue(static_cast<unsigned short>(pixel_value));

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

inline Image * tissuestack::imaging::UncachedImageExtraction::getImageTile(
		Image * img,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	if (img == NULL) return NULL;

	// check if we don't exceed bounds
	unsigned int xOffset = request->getXCoordinate()*request->getLengthOfSquare();
	unsigned int yOffset = request->getYCoordinate()*request->getLengthOfSquare();

	if (xOffset >  img->columns || yOffset > img->rows)
	{
		DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image Extraction: tile number(x/y) exceeds the width/height of the image (given the square length)");
	}

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	RectangleInfo * tile = static_cast<RectangleInfo *>(malloc(sizeof(*tile)));
	tile->height = request->getLengthOfSquare();
	tile->width = request->getLengthOfSquare();
	tile->x = xOffset;
	tile->y = yOffset;

	Image * tmp = img;
	img = CropImage(img, tile, &exception);

	free(tile);
	DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to crop image to get tile!");
	}

	return img;
}
