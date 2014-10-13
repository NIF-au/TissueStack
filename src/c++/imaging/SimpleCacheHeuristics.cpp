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
	const tissuestack::imaging::TissueStackRawData * image,
	const tissuestack::networking::TissueStackQueryRequest * request) const
{
	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	if (request->getXCoordinate() > actualDimension->getWidth() ||
			request->getYCoordinate() > actualDimension->getHeight())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image Query: Coordinate (x/y) exceeds the width/height of the image slice!");

	const unsigned char * cache_data =
		this->findCacheHit(image, request);

	bool wasAddedToCache = true;

	// TODO: do cache stats (perhaps include it in findCacheHit as increment)
	if (cache_data == nullptr)
	{
		cache_data = this->_uncached_extraction->extractImageOnly(image, request);
		if (cache_data == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract image data");

		// potentially add to cache store
		wasAddedToCache =
			const_cast<tissuestack::imaging::SimpleCacheHeuristics *>(this)->addToCache(
				cache_data, image, request);
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

	// delete if not cached
	if (!wasAddedToCache) delete [] cache_data;

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
	const unsigned char * cache_data =
		this->findCacheHit(image, request);

	bool wasAddedToCache = true;

	// TODO: do cache stats (perhaps include it in findCacheHit as increment)
	if (cache_data == nullptr)
	{
		cache_data = this->_uncached_extraction->extractImageOnly(image, request);
		if (cache_data == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract image data");

		// potentially add to cache store
		wasAddedToCache =
			const_cast<tissuestack::imaging::SimpleCacheHeuristics *>(this)->addToCache(
				cache_data, image, request);
	}

	const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			image->getDimensionByLongName(request->getDimensionName());

	Image * img =
		this->_uncached_extraction->createImageFromDataRead(image, actualDimension, cache_data);

	// delete if not cached
	if (!wasAddedToCache) delete [] cache_data;

	return img;
}

const unsigned char * tissuestack::imaging::SimpleCacheHeuristics::findCacheHit(
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	//TODO: has to be mutexed
	// TODO: implement
	return nullptr;
}

const bool tissuestack::imaging::SimpleCacheHeuristics::addToCache(
	const unsigned char * data,
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request)
{
	//TODO: has to be mutexed
	// TODO: implement
	return false;
}
