#include "imaging.h"

tissuestack::imaging::Image::~Image()
{
	this->closeFileHandle();
}

tissuestack::imaging::Image::Image(const std::string filename)
	: tissuestack::imaging::Image::Image(
			filename,
			tissuestack::imaging::FORMAT::RAW) {}

tissuestack::imaging::Image::Image(
		const std::string filename,
		const tissuestack::imaging::FORMAT format) :
			_file_name(filename), _format(format) {}

void tissuestack::imaging::Image::closeFileHandle()
{
	if (this->_file_handle)
	{
		fclose(this->_file_handle);
		this->_file_handle = nullptr;
	}
}

const std::string tissuestack::imaging::Image::getFileName() const
{
	return this->_file_name;
}

const tissuestack::imaging::Dimension * const tissuestack::imaging::Image::getDimensionByLongName(const std::string dimension) const
{
	if (dimension.empty()) return nullptr;

	return this->getDimension(dimension.at(0));
}

const tissuestack::imaging::Dimension * const tissuestack::imaging::Image::getDimension(const char dimension_letter) const
{
	try
	{
		return &(this->_dimensions.at(dimension_letter));

	} catch (std::out_of_range & not_found) {
		return nullptr;
	}
}

const long long int tissuestack::imaging::Image::getGlobalMinumum() const
{
	return this->_global_min_value;
}

const long long int tissuestack::imaging::Image::getGlobalMaximum() const
{
	return this->_global_max_value;
}

const tissuestack::imaging::FORMAT tissuestack::imaging::Image::getFormat() const
{
	return this->_format;
}
