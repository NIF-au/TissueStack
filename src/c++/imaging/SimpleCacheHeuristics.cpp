#include "imaging.h"

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
	const TissueStackRawData * image,
	const tissuestack::networking::TissueStackImageRequest * request) const
{
	// for now just delegate to uncached extraction
	// TODO: implement cache
	return this->_uncached_extraction->extractImage(image, request);
}
