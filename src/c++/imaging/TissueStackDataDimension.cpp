#include "imaging.h"

tissuestack::imaging::TissueStackDataDimension::TissueStackDataDimension(
		const std::string & name,
		const unsigned long long int offset,
		const unsigned long long int number_of_slices,
		const unsigned long long int slice_size) :
		_id(0), _name(name), _offset(offset), _numberOfSlices(number_of_slices),_sliceSize(slice_size), _width(0), _height(0)
{
	if (name.empty() || offset <=0 || number_of_slices <=0 || slice_size <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "DataDimension Initialization with a fishy number smaller or equal to 0!");
}

tissuestack::imaging::TissueStackDataDimension::TissueStackDataDimension(
		const long long unsigned int id,
		const std::string & name,
		const unsigned long long int number_of_slices) : _id(id), _name(name), _offset(0), _numberOfSlices(number_of_slices), _sliceSize(0), _width(0), _height(0)
{
	if (name.empty() || number_of_slices <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "DataDimension Initialization with a fishy number smaller or equal to 0!");
}

const std::string tissuestack::imaging::TissueStackDataDimension::getName() const
{
	return this->_name;
}

void tissuestack::imaging::TissueStackDataDimension::setWidthAndHeight(const std::array<unsigned int, 2> & widthAndHeight)
{
	if (widthAndHeight[0] <=0 || widthAndHeight[1] <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"DataDimension Initialization with an invalid width/height of less than or equal to 0");
	this->_width = widthAndHeight[0];
	this->_height = widthAndHeight[1];
}

const unsigned int tissuestack::imaging::TissueStackDataDimension::getWidth() const
{
	return this->_width;
}

const unsigned int tissuestack::imaging::TissueStackDataDimension::getHeight() const
{
	return this->_height;
}

const unsigned long long int tissuestack::imaging::TissueStackDataDimension::getNumberOfSlices() const
{
	return this->_numberOfSlices;
}

const unsigned long long int tissuestack::imaging::TissueStackDataDimension::getSliceSize() const
{
	return this->_sliceSize;
}

const unsigned long long int tissuestack::imaging::TissueStackDataDimension::getOffset() const
{
	return this->_offset;
}

void tissuestack::imaging::TissueStackDataDimension::dumpDataDimensionInfoIntoDebugLog() const
{
	std::ostringstream in;

	in << "Dimension: " << this->_name << " => # " << this->_numberOfSlices
			<< " (" << this->_sliceSize << " px) @ " << this->_offset;

	tissuestack::logging::TissueStackLogger::instance()->debug("%s\n", in.str().c_str());
}
