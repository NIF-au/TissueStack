#include "networking.h"
#include "imaging.h"

const bool tissuestack::imaging::TissueStackMincData::isRaw() const
{
	return false;
}

tissuestack::imaging::TissueStackMincData::TissueStackMincData(const std::string & filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::MINC)
{
	// TODO: check mincness and set other properties
}

tissuestack::imaging::TissueStackMincData::~TissueStackMincData()
{
	// do nothing for now
}
