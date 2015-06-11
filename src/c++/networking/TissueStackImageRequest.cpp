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

const std::string tissuestack::networking::TissueStackImageRequest::SERVICE1 = "IMAGE";
const std::string tissuestack::networking::TissueStackImageRequest::SERVICE2 = "IMAGE_PREVIEW";

tissuestack::networking::TissueStackImageRequest::TissueStackImageRequest() {}

void tissuestack::networking::TissueStackImageRequest::setDataSetFromRequestParameters(
		const std::unordered_map<std::string, std::string> & request_parameters)
{
	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "dataset");
	if (value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'dataset' was not supplied!");

	this->_datasets = tissuestack::utils::Misc::tokenizeString(value, ':');
	if (this->_datasets.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'dataset' was not supplied!");

	for (auto ds : this->_datasets)
		if (!tissuestack::utils::System::fileExists(ds))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Parameter 'dataset' does not represent an existing file!");
}

void tissuestack::networking::TissueStackImageRequest::setTimeStampInfoFromRequestParameters(
		const std::unordered_map<std::string, std::string> & request_parameters)
{
	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "id");
	if (!value.empty())
	{
		try
		{
			this->_request_id = strtoull(value.c_str(), NULL, 10);
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'id' is not a valid positve integer!");
		}
	}

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "timestamp");
	if (!value.empty())
	{
		try
		{
			this->_request_timestamp = strtoull(value.c_str(), NULL, 10);
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'timestamp' is not a valid positve integer!");
		}
	}

}

void tissuestack::networking::TissueStackImageRequest::setDimensionFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters)
{
	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "dimension");
	if (value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'dimension' was not supplied!");
	this->_dimension_name = value;

}

void tissuestack::networking::TissueStackImageRequest::setSliceFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters)
{
	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "slice");
	if (value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'slice' was not supplied!");
	try
	{
		this->_slice_number =  static_cast<unsigned int>(strtoul(value.c_str(), NULL, 10));
	} catch (...)
	{
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'slice' is not a valid positve integer!");
	}
}

void tissuestack::networking::TissueStackImageRequest::setCoordinatesFromRequestParameters(
		const std::unordered_map<std::string, std::string> & request_parameters, const bool is_preview)
{
	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "x");
	if (!is_preview && value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'x' was not supplied!");

	if (!value.empty())
	{
		try
		{
			this->_x_coordinate = static_cast<unsigned int>(strtoul(value.c_str(), NULL, 10));
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'x' is not a valid positve integer!");
		}
	}

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "y");
	if (!is_preview && value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'y' was not supplied!");

	if (!value.empty())
	{
		try
		{
			this->_y_coordinate = static_cast<unsigned int>(strtoul(value.c_str(), NULL, 10));
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Mandatory parameter 'y' is not a valid positve integer!");
		}
	}
}

void tissuestack::networking::TissueStackImageRequest::setImageRequestMembersFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters)
{
	this->setTimeStampInfoFromRequestParameters(request_parameters);
	this->setDataSetFromRequestParameters(request_parameters);
	this->setDimensionFromRequestParameters(request_parameters);
	this->setSliceFromRequestParameters(request_parameters);

	std::string value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "scale");
	if (!value.empty())
	{
		try
		{
			this->_scale_factor = strtof(value.c_str(), NULL);
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'scale' is not a valid floating point number!");
		}
	}

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "quality");
	if (!value.empty())
	{
		try
		{
			this->_quality_factor = strtof(value.c_str(), NULL);
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'quality' is not a valid floating point number!");
		}
	}

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "image_type");
	if (!value.empty())
		this->_output_image_format = value;

	std::transform(this->_output_image_format.begin(), this->_output_image_format.end(), this->_output_image_format.begin(), toupper);

	if (this->_output_image_format.compare("PNG") != 0 && this->_output_image_format.compare("JPEG") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Parameter 'image_type' can only be 'PNG' or 'JPEG'!");

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "colormap");
	if (!value.empty())
		this->_color_map_name = value;

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "min");
	if (!value.empty())
	{
		try
		{
			this->_contrast_min = static_cast<unsigned short>(atoi(value.c_str()));
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'min' is not a valid positve integer!");
		}
	}

	value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "max");
	if (!value.empty())
	{
		try
		{
			this->_contrast_max = static_cast<unsigned short>(atoi(value.c_str()));
		} catch (...)
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'max' is not a valid positve integer!");
		}
	}

	// the tile coordinates and the square length are not meaningful for previews
	this->setCoordinatesFromRequestParameters(request_parameters, this->_is_preview);
	if (!this->_is_preview)
	{
		value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "square");
		if (!value.empty())
		{
			try
			{
				this->_length_of_square = strtoul(value.c_str(), NULL, 10);
			} catch (...)
			{
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'square' is not a valid positive integer!");
			}
		}
	} else
	{
		// optional width and height
		value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "width");
		if (!value.empty())
		{
			try
			{
				this->_width = strtoul(value.c_str(), NULL, 10);
			} catch (...)
			{
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'width' is not a valid positive integer!");
			}
		}
		value = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "height");
		if (!value.empty())
		{
			try
			{
				this->_height = strtoul(value.c_str(), NULL, 10);
			} catch (...)
			{
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Optional Parameter 'height' is not a valid positive integer!");
			}
		}
	}

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_IMAGE);
}

tissuestack::networking::TissueStackImageRequest::TissueStackImageRequest(
		std::unordered_map<std::string, std::string> & request_parameters, bool is_preview) : _is_preview(is_preview)
{
	this->setImageRequestMembersFromRequestParameters(request_parameters);
}

tissuestack::networking::TissueStackImageRequest::TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters)
{
	// set flag if we only want a prview
	std::string service = tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "SERVICE");
	std::transform(service.begin(), service.end(), service.begin(), toupper);
	if (tissuestack::networking::TissueStackImageRequest::SERVICE2.compare(service) == 0)
		this->_is_preview = true;

	this->setImageRequestMembersFromRequestParameters(request_parameters);
}

const bool tissuestack::networking::TissueStackImageRequest::isObsolete() const
{
	return this->hasExpired();
}

const bool tissuestack::networking::TissueStackImageRequest::hasExpired() const
{
	// convenience method that unlike isObsolete does not add new requests and is inlined for frequent calling
	return tissuestack::common::RequestTimeStampStore::instance()->checkForExpiredEntry(this->_request_id, this->_request_timestamp);
}

const std::string tissuestack::networking::TissueStackImageRequest::getContent() const
{
	return std::string("TS_IMAGE");
}

const std::string tissuestack::networking::TissueStackImageRequest::getColorMapName() const
{
	return this->_color_map_name;
}

const std::vector<std::string> tissuestack::networking::TissueStackImageRequest::getDataSetLocations() const
{
	return this->_datasets;
}

const std::string tissuestack::networking::TissueStackImageRequest::getDimensionName() const
{
	return this->_dimension_name;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getWidth() const
{
	return this->_width;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getHeight() const
{
	return this->_height;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getLengthOfSquare() const
{
	return this->_length_of_square;
}

const unsigned short tissuestack::networking::TissueStackImageRequest::getContrastMinimum() const
{
	return this->_contrast_min;
}

const unsigned short tissuestack::networking::TissueStackImageRequest::getContrastMaximum() const
{
	return this->_contrast_max;
}
const bool tissuestack::networking::TissueStackImageRequest::isPreview() const
{
	return this->_is_preview;
}

const std::string tissuestack::networking::TissueStackImageRequest::getOutputImageFormat() const
{
	return this->_output_image_format;
}

const float tissuestack::networking::TissueStackImageRequest::getQualityFactor() const
{
	return this->_quality_factor;
}

const float tissuestack::networking::TissueStackImageRequest::getScaleFactor() const
{
	return this->_scale_factor;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getSliceNumber() const
{
	return this->_slice_number;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getXCoordinate() const
{
	return this->_x_coordinate;
}

const unsigned int tissuestack::networking::TissueStackImageRequest::getYCoordinate() const
{
	return this->_y_coordinate;
}

