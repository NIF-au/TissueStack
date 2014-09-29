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
	// delegate
	// TODO: implement cache
	return this->_uncached_extraction->performQuery(image, request);
}

const Image *  tissuestack::imaging::SimpleCacheHeuristics::extractImage(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	// for now just delegate to uncached extraction
	// TODO: implement cache
	return this->_uncached_extraction->extractImage(processing_strategy, image, request);
}
