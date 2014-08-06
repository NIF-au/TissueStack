#include "imaging.h"

const bool tissuestack::imaging::TissueStackNiftiData::isRaw()
{
	return false;
}

const bool tissuestack::imaging::TissueStackNiftiData::isColor()
{
	return this->_is_color;
}

tissuestack::imaging::TissueStackNiftiData::TissueStackNiftiData(const std::string & filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::NIFTI)
{
	// TODO: check niftiness and set other properties
}

tissuestack::imaging::TissueStackNiftiData::~TissueStackNiftiData()
{
	// do nothing for now
}