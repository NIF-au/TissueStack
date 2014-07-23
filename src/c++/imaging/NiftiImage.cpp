#include "imaging.h"

const bool tissuestack::imaging::NiftiImage::isRaw()
{
	return false;
}

tissuestack::imaging::NiftiImage::NiftiImage(const std::string filename) :
		tissuestack::imaging::Image(filename, tissuestack::imaging::FORMAT::NIFTI)
{
	// TODO: set other properties
}
