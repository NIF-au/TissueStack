#include "imaging.h"

const bool tissuestack::imaging::TissueStackDataBaseData::isRaw() const
{
	return false;
}

tissuestack::imaging::TissueStackDataBaseData::TissueStackDataBaseData(const unsigned long long int id) :
		tissuestack::imaging::TissueStackImageData(id) {}

tissuestack::imaging::TissueStackDataBaseData::~TissueStackDataBaseData()
{
	// do nothing for now
}
