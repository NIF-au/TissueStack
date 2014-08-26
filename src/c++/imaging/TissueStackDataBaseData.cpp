#include "imaging.h"

const bool tissuestack::imaging::TissueStackDataBaseData::isRaw() const
{
	return false;
}

tissuestack::imaging::TissueStackDataBaseData::TissueStackDataBaseData(
		const unsigned long long int id, const std::string filename) :
		tissuestack::imaging::TissueStackImageData(id, filename) {}

tissuestack::imaging::TissueStackDataBaseData::~TissueStackDataBaseData()
{
	// do nothing for now
}
