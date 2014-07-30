#include "imaging.h"

tissuestack::imaging::TissueStackDataDimension::TissueStackDataDimension(const std::string name) : _name(name) {}

const std::string tissuestack::imaging::TissueStackDataDimension::getName() const
{
	return this->_name;
}

const unsigned long long int tissuestack::imaging::TissueStackDataDimension::getNumberOfSlices() const
{
	return this->_slices;
}

const unsigned long long int tissuestack::imaging::TissueStackDataDimension::getOffset() const
{
	return this->_offset;
}

const int tissuestack::imaging::TissueStackDataDimension::getMinumum() const
{
	return this->_min_value;
}

const int tissuestack::imaging::TissueStackDataDimension::getMaximum() const
{
	return this->_max_value;
}
