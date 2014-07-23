#include "imaging.h"

const bool tissuestack::imaging::RawImage::isRaw()
{
	return true;
}

tissuestack::imaging::RawImage::RawImage(const std::string filename) :
		tissuestack::imaging::Image(filename, tissuestack::imaging::FORMAT::RAW)
{
	// TODO: set other properties
}
