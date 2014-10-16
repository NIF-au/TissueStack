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
#include "database.h"

tissuestack::imaging::SimpleCacheHeuristics::SimpleCacheHeuristics() :
	tissuestack::imaging::SimpleCacheHeuristics::SimpleCacheHeuristics(new tissuestack::imaging::UncachedImageExtraction()) {}

tissuestack::imaging::SimpleCacheHeuristics::SimpleCacheHeuristics(
		const tissuestack::imaging::UncachedImageExtraction * image_extraction) :
_uncached_extraction(image_extraction)
{
	if (image_extraction == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Cannot instantiate a Caching Strategy without an Extraction Instance!");
}

tissuestack::imaging::SimpleCacheHeuristics::~SimpleCacheHeuristics()
{
	if (this->_uncached_extraction)
		delete this->_uncached_extraction;
}


const std::array<unsigned long long int, 3> tissuestack::imaging::SimpleCacheHeuristics::performQuery(
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

	bool needsToBeAddedToCache = false;

	const unsigned char * cache_data =
		this->findCacheHit(image, request);

	if (cache_data == nullptr)
	{
		needsToBeAddedToCache = true;
		cache_data = this->_uncached_extraction->extractImageOnly(image, request);
		if (cache_data == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract image data");
	}

	std::array<unsigned long long int, 3> pixel_value;
	if ((image->getRawVersion() == tissuestack::imaging::RAW_FILE_VERSION::LEGACY &&
			image->getFormat() == tissuestack::imaging::FORMAT::RAW) ||
			image->getRawVersion() == tissuestack::imaging::RAW_FILE_VERSION::V1)
	{
		unsigned long long int multiplier = 1;
		if (image->getType() != tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT)
			multiplier = 3;

		// set position within slice
		unsigned long long int actualOffset =
			static_cast<unsigned long long int>(
					static_cast<unsigned long long int>(
						request->getYCoordinate())*actualDimension->getWidth()*multiplier +
					static_cast<unsigned long long int>(request->getXCoordinate()*multiplier));

		pixel_value[0] = static_cast<unsigned long long int>(cache_data[actualOffset]);
		pixel_value[1] = static_cast<unsigned long long int>(cache_data[actualOffset+1]);
		pixel_value[2] = static_cast<unsigned long long int>(cache_data[actualOffset+2]);
	} else
	{
		Image * img =
			this->_uncached_extraction->createImageFromDataRead(image, actualDimension, cache_data);
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
			pixel_value[0] = this->_uncached_extraction->mapUnsignedValue(img->depth, 8, pixel_value[0]);
			pixel_value[1] = this->_uncached_extraction->mapUnsignedValue(img->depth, 8, pixel_value[1]);
			pixel_value[2] = this->_uncached_extraction->mapUnsignedValue(img->depth, 8, pixel_value[2]);
		}
		DestroyImage(img);
	}

	if (needsToBeAddedToCache)
		this->addToCache(
			processing_strategy,
			image,
			request,
			cache_data);

	return pixel_value;
}

Image * tissuestack::imaging::SimpleCacheHeuristics::applyPostExtractionTasks(
		Image * img,
		const tissuestack::imaging::TissueStackRawData * image,
		const tissuestack::networking::TissueStackImageRequest * request) const
{
	return this->_uncached_extraction->applyPostExtractionTasks(img, image, request);
}

const Image *  tissuestack::imaging::SimpleCacheHeuristics::extractImage(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	bool needsToBeAddedToCache = false;

	const unsigned char * cache_data =
		this->findCacheHit(image, request);

	if (cache_data == nullptr)
	{
		needsToBeAddedToCache = true;
		cache_data = this->_uncached_extraction->extractImageOnly(image, request);
		if (cache_data == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract image data");
	}

	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	Image * img =
		this->_uncached_extraction->createImageFromDataRead(image, actualDimension, cache_data);


	if (needsToBeAddedToCache)
		this->addToCache(
			processing_strategy,
			image,
			request,
			cache_data);

	return img;
}

void tissuestack::imaging::SimpleCacheHeuristics::addToCache(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request,
	const unsigned char * data) const
{

	const std::string dataset = image->getFileName();

	unsigned long int slice = 0;

	for (auto dim : image->getDimensionOrder())
	{
		if (request->getDimensionName().at(0) == dim.at(0))
			break;

		slice += image->getDimensionByLongName(dim)->getNumberOfSlices();
	}
	slice += request->getSliceNumber();

	if (!tissuestack::imaging::TissueStackSliceCache::instance()->addCacheEntry(
							dataset, slice, data))
		delete [] data;
}

const unsigned char * tissuestack::imaging::SimpleCacheHeuristics::findCacheHit(
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	unsigned long int slice = 0;

	for (auto dim : image->getDimensionOrder())
	{
		if (request->getDimensionName().at(0) == dim.at(0))
			break;

		slice += image->getDimensionByLongName(dim)->getNumberOfSlices();
	}
	slice += request->getSliceNumber();

	return
		tissuestack::imaging::TissueStackSliceCache::instance()->findCacheEntry(
			image->getFileName(), slice);
}
