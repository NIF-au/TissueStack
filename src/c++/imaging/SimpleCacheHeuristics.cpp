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

void tissuestack::imaging::SimpleCacheHeuristics::extractImage(
						const TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	// delegate
	this->extractImage(-1, image, request, slice);
}

void tissuestack::imaging::SimpleCacheHeuristics::extractImage(
						const int descriptor,
						const TissueStackRawData * image,
						const tissuestack::networking::TissueStackImageRequest * request,
						const unsigned long long int slice) const
{
	// for now just delegate to uncached extraction
	this->_uncached_extraction->extractImage(descriptor, image, request, slice);
}
