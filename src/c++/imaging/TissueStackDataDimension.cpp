/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "networking.h"
#include "imaging.h"

tissuestack::imaging::TissueStackDataDimension::TissueStackDataDimension(
		const std::string & name,
		const unsigned long long int offset,
		const unsigned long long int number_of_slices,
		const unsigned long long int slice_size) :
		_id(0), _name(name), _offset(offset), _numberOfSlices(number_of_slices),_sliceSize(slice_size), _width(0), _height(0)
{
	if (name.empty() || number_of_slices <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "DataDimension Initialization with either 0 slices or empty name or both!");
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

void tissuestack::imaging::TissueStackDataDimension::setWidthAndHeight(
	const unsigned int width, const unsigned int height,
	const unsigned int anisotropic_width, const unsigned int anisotropic_height)
{
	if (width <=0 || height <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"DataDimension Initialization with an invalid width/height of less than or equal to 0");
	this->_width = width;
	this->_height = height;
	this->_anisotropic_width = anisotropic_width < 1 ? 1 : anisotropic_width;
	this->_anisotropic_height = anisotropic_height < 1 ? 1 : anisotropic_height;
}

void tissuestack::imaging::TissueStackDataDimension::setTransformationMatrix(const std::string transformationMatrix)
{
	this->_transformationMatrix = transformationMatrix;
}

void tissuestack::imaging::TissueStackDataDimension::initialize2DData(
		const std::vector<float> & coords, const std::vector<float> & steps)
{
	this->setIsotropyFactor(1);
	const std::string defaultMatrix = "[[1,0,0],[0,1,0],[0,0,1]]";
	if (coords.size() != 2 &&  steps.size() != 2)
	{
		this->setTransformationMatrix(defaultMatrix);
		return;
	}

	std::ostringstream matrix;
	matrix << "[[";
	matrix << std::to_string(steps[0]) << ",0,"
		<< std::to_string(coords[0]) << "],";
	matrix << "[0," << std::to_string(steps[1]) << ","
		<< std::to_string(coords[1]) << "],";
	matrix << "[0,0,1]]";

	this->setTransformationMatrix(matrix.str());
}

const std::string tissuestack::imaging::TissueStackDataDimension::getTransformationMatrix() const
{
	return this->_transformationMatrix;
}

const unsigned int tissuestack::imaging::TissueStackDataDimension::getWidth() const
{
	return this->_width;
}

const unsigned int tissuestack::imaging::TissueStackDataDimension::getAnisotropicWidth() const
{
	return this->_anisotropic_width;
}


const unsigned int tissuestack::imaging::TissueStackDataDimension::getHeight() const
{
	return this->_height;
}

const unsigned int tissuestack::imaging::TissueStackDataDimension::getAnisotropicHeight() const
{
	return this->_anisotropic_height;
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

	in << "Dimension: " << this->_name << " => # " << this->getNumberOfSlices()
			<< " (" << this->_sliceSize << " px) @ " << this->_offset;

	tissuestack::logging::TissueStackLogger::instance()->debug("%s\n", in.str().c_str());
}

void tissuestack::imaging::TissueStackDataDimension::setSliceSizeFromGivenWidthAndHeight()
{
	this->_sliceSize = this->_height * this->_width;
}

void tissuestack::imaging::TissueStackDataDimension::setOffSet(const unsigned long long int offSet)
{
	this->_offset = offSet;
}

const float tissuestack::imaging::TissueStackDataDimension::getIsotropyFactor() const
{
	return this->_isotropy_factor;
}

void tissuestack::imaging::TissueStackDataDimension::setIsotropyFactor(const float isotropy_factor)
{
	this->_isotropy_factor = isotropy_factor;
}

