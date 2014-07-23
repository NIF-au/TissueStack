#include "imaging.h"

const bool tissuestack::imaging::MincImage::isRaw()
{
	return false;
}

tissuestack::imaging::MincImage::MincImage(const std::string filename) :
		tissuestack::imaging::Image(filename, tissuestack::imaging::FORMAT::MINC)
{
	// TODO: set other properties
}
