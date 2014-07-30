#include "imaging.h"

const bool tissuestack::imaging::TissueStackRawData::isRaw()
{
	return true;
}

tissuestack::imaging::TissueStackRawData::TissueStackRawData(const std::string filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::RAW)
{
	// check if we've got a raw file in front of us ...
}
