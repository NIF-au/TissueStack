#include "imaging.h"

tissuestack::imaging::Dimension::Dimension(const std::string name) : _name(name) {}

const std::string tissuestack::imaging::Dimension::getName() const
{
	return this->_name;
}

const unsigned int tissuestack::imaging::Dimension::getNumberOfSlices() const
{
	return this->_slices;
}

const long long int tissuestack::imaging::Dimension::getMinumum() const
{
	return this->_min_value;
}

const long long int tissuestack::imaging::Dimension::getMaximum() const
{
	return this->_max_value;
}
