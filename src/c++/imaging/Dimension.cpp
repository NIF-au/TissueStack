#include "imaging.h"

tissuestack::imaging::Dimension::Dimension(const std::string name) : _name(name) {}

const std::string tissuestack::imaging::Dimension::getName()
{
	return this->_name;
}

const unsigned int tissuestack::imaging::Dimension::getNumberOfSlices()
{
	return this->_slices;
}

const long long int tissuestack::imaging::Dimension::getMinumum()
{
	return this->_min_value;
}

const long long int tissuestack::imaging::Dimension::getMaximum()
{
	return this->_max_value;
}
