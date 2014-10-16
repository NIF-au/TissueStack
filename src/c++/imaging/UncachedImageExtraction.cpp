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

tissuestack::imaging::UncachedImageExtraction::UncachedImageExtraction()
{

}

inline unsigned char * tissuestack::imaging::UncachedImageExtraction::readRawSlice(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned int sliceNumber) const
{
	unsigned long long int multiplier = 1;
	if (image->getType() != tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT)
		multiplier = 3;

	long long int dataLength =
			actualDimension->getSliceSize() * multiplier;
	unsigned long long int actualOffset =
			actualDimension->getOffset() +
				static_cast<unsigned long long int>(sliceNumber) * static_cast<unsigned long long int>(dataLength);

	// read the actual raw data to write out images later on
	unsigned char * data = new unsigned char[dataLength];
	const int fd =
		const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor();
	lseek(
		fd,
		actualOffset,
		SEEK_SET);
	ssize_t bRead =
		read(
			fd,
			static_cast<void *>(data),
			dataLength);

	if (bRead != dataLength)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to read entire slice from RAW file!");

	return data;
}

const unsigned char * tissuestack::imaging::UncachedImageExtraction::extractImageOnly(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	// determine some parameters for data reading
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	unsigned char * data =
		this->readRawSlice(image, actualDimension, request->getSliceNumber());

	if (request->hasExpired())
	{
		delete [] data;
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");
	}

	return data;
}

Image * tissuestack::imaging::UncachedImageExtraction::extractImageForPreTiling(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned int sliceNumber) const
{
	unsigned char * data =
		this->readRawSlice(
				image,
				actualDimension,
				sliceNumber);

	return
		this->createImageFromDataRead0(
			image,
			actualDimension,
			data);
}

Image * tissuestack::imaging::UncachedImageExtraction::applyPreTilingProcessing(
	Image * img,
	const std::string color_map_name,
	unsigned long int & width,
	unsigned long int & height,
	const float scaleFactor) const
{
	if (img == nullptr)
		return nullptr;

	// perform color mapping if requested
	if (color_map_name.compare("gray") != 0 &&
		color_map_name.compare("grey") != 0)
	{
		img = this->convertAnythingToRgbImage(img);
		this->applyColorMap(
				img,
				color_map_name,
				width,
				height);
	}

	// adjust scale (if requested)
	if (img && scaleFactor != static_cast<const float>(1.0))
	{
		width =
			static_cast<unsigned int>(
				static_cast<const float>(width) * scaleFactor);
		height =
			static_cast<unsigned int>(
					static_cast<const float>(height) * scaleFactor);
		img =
			this->scaleImage(
				img,
				width,
				height);
	}

	return img;
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

	// timeout/shutdown check
	if (request->hasExpired())
	{
		if (img) DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");
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

	// timeout/shutdown check
	if (request->hasExpired())
	{
		if (img) DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");
	}

	// adjust scale (if requested)
	if (request->getScaleFactor() != static_cast<const float>(1.0))
		img =
			this->scaleImage(
				img,
				static_cast<const float>(actualDimension->getWidth()) * request->getScaleFactor(),
				static_cast<const float>(actualDimension->getHeight()) * request->getScaleFactor());

	// timeout/shutdown check
	if (request->hasExpired())
	{
		if (img) DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"Old Image Request!");
	}

	// adjust quality (if requested)
	if (request->getQualityFactor() < static_cast<const float>(1.0))
		img =
			this->degradeImage0(
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
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::imaging::TissueStackRawData * image,
	const tissuestack::networking::TissueStackQueryRequest * request) const
{
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	if (request->getXCoordinate() > actualDimension->getWidth() ||
			request->getYCoordinate() > actualDimension->getHeight())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image Query: Coordinate (x/y) exceeds the width/height of the image slice!");

	std::array<unsigned long long int, 3> pixel_value;
	if ((image->getRawVersion() == tissuestack::imaging::RAW_FILE_VERSION::LEGACY &&
			image->getFormat() == tissuestack::imaging::FORMAT::RAW) ||
			image->getRawVersion() == tissuestack::imaging::RAW_FILE_VERSION::V1)
	{

		unsigned long long int multiplier = 1;
		if (image->getType() != tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT)
			multiplier = 3;
		long long int dataLength =
				actualDimension->getSliceSize() * multiplier;
		// set slice offset
		unsigned long long int actualOffset =
				actualDimension->getOffset() +
					static_cast<unsigned long long int>(request->getSliceNumber()) * static_cast<unsigned long long int>(dataLength);
		// set position within slice
		actualOffset +=
			static_cast<unsigned long long int>(
					static_cast<unsigned long long int>(
						request->getYCoordinate())*actualDimension->getWidth()*multiplier +
					static_cast<unsigned long long int>(request->getXCoordinate()*multiplier));

		unsigned char * data = new unsigned char[3] {'\0', '\0', '\0'};
		const int fd =
			const_cast<tissuestack::imaging::TissueStackRawData *>(image)->getFileDescriptor();
		lseek(
			fd,
			actualOffset,
			SEEK_SET);
		ssize_t bRead =
			read(
				fd,
				static_cast<void *>(data),
				3);

		if (bRead != 3)
		{
			delete [] data;
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Failed to query slice within RAW file!");
		}

		pixel_value[0] = static_cast<unsigned long long int>(data[0]);
		pixel_value[1] = static_cast<unsigned long long int>(data[1]);
		pixel_value[2] = static_cast<unsigned long long int>(data[2]);

		return pixel_value;
	}

	Image * img =
		this->extractImage(image, request);

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

Image * tissuestack::imaging::UncachedImageExtraction::extractImage(
	const tissuestack::imaging::TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	const unsigned char * data =
		this->extractImageOnly(
			image,
			request);
	if (data == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not extract image data");

	// determine some parameters for data reading
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	Image * img =
		this->createImageFromDataRead0(
		image,
		actualDimension,
		data);
	if (img == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create Image");

	return img;
}

Image * tissuestack::imaging::UncachedImageExtraction::createImageFromDataRead(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned char * data) const
{
	return this->createImageFromDataRead0(image, actualDimension, data);
}

inline Image * tissuestack::imaging::UncachedImageExtraction::createImageFromDataRead0(
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::imaging::TissueStackDataDimension * actualDimension,
		const unsigned char * data) const
{
	Image * img = NULL;
	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	const std::vector<std::string> dim_order = image->getDimensionOrder();
	const char dim = actualDimension->getName().at(0);

	if (data == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Data is null!");

	img = ConstituteImage(
		actualDimension->getWidth(),
		actualDimension->getHeight(),
		(image->getType() == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT) ? "I" : "RGB",
		CharPixel,
		data, &exception);

	// sanity check: was graphics magick able to create an image based on what we gave it?
	if (img == NULL)
	{
		delete [] data;
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}

	if (image->getFormat() == tissuestack::imaging::FORMAT::RAW) return img;

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
			delete [] data;
			CatchException(&exception);
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
			delete [] data;
			CatchException(&exception);
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Image Extraction: Failed to flop image to make it backward compatible!");
		}
	}

	return img;
}

Image * tissuestack::imaging::UncachedImageExtraction::degradeImage(
		Image * img,
		const unsigned int width,
		const unsigned int height,
		const float quality_factor) const
{
	return this->degradeImage0(img, width, height, quality_factor);
}

inline Image * tissuestack::imaging::UncachedImageExtraction::degradeImage0(
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to scale image!");

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

unsigned long long tissuestack::imaging::UncachedImageExtraction::mapUnsignedValue(const unsigned char fromBitRange, const unsigned char toBitRange, const unsigned long long value) const
{
	return this->mapUnsignedValue0(fromBitRange, toBitRange, value);
}
inline unsigned long long tissuestack::imaging::UncachedImageExtraction::mapUnsignedValue0(const unsigned char fromBitRange, const unsigned char toBitRange, const unsigned long long value) const {
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
		if (img) DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Colormap Application: Could not find color map!");
	}

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	PixelPacket * pixels = GetImagePixelsEx(img, 0, 0, width, height, &exception);
	if (pixels == NULL)
	{
		CatchException(&exception);
		if (img) DestroyImage(img);
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

inline Image * tissuestack::imaging::UncachedImageExtraction::getImageTile0(
		Image * img,
		const unsigned int xCoordinate,
		const unsigned int yCoordinate,
		const unsigned int squareLength,
		const bool keepOriginalIntact) const
{
	if (img == NULL) return NULL;

	// check if we don't exceed bounds
	unsigned int xOffset = xCoordinate * squareLength;
	unsigned int yOffset = yCoordinate * squareLength;

	if (xOffset >  img->columns || yOffset > img->rows)
	{
		DestroyImage(img);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image Extraction: tile number(x/y) exceeds the width/height of the image (given the square length)");
	}

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	RectangleInfo * tile = static_cast<RectangleInfo *>(malloc(sizeof(*tile)));
	tile->height = squareLength;
	tile->width = squareLength;
	tile->x = xOffset;
	tile->y = yOffset;

	Image * tmp = img;
	img = CropImage(img, tile, &exception);

	free(tile);
	if (!keepOriginalIntact) DestroyImage(tmp);
	if (img == NULL)
	{
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Image Extraction: Failed to crop image to get tile!");
	}

	return img;
}

Image * tissuestack::imaging::UncachedImageExtraction::getImageTileForPreTiling(
		Image * img,
		const unsigned int xCoordinate,
		const unsigned int yCoordinate,
		const unsigned int squareLength) const
{
	if (img == NULL) return NULL;

	// delegate
	return this->getImageTile0(
		img,
		xCoordinate,
		yCoordinate,
		squareLength,
		true);
}


inline Image * tissuestack::imaging::UncachedImageExtraction::getImageTile(
		Image * img,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	if (img == NULL) return NULL;

	// delegate
	return this->getImageTile0(
		img,
		request->getXCoordinate(),
		request->getYCoordinate(),
		request->getLengthOfSquare(),
		false);
}
