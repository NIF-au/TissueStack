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
#include "database.h"

tissuestack::imaging::TissueStackImageData::TissueStackImageData(
		const unsigned long long int id, const std::string filename) :
	_file_name(filename), _format(tissuestack::imaging::FORMAT::DATABASE), _database_id(id), _is_tiled(false), _resolutionMm(0) {}

tissuestack::imaging::TissueStackImageData::~TissueStackImageData()
{
	this->closeFileHandle();
	for (auto dim : this->_dimensions)
		if (dim.second) delete dim.second;
	this->_dimensions.clear();
}

void tissuestack::imaging::TissueStackImageData::closeFileHandle()
{
	if (this->_file_handle)
	{
		fclose(this->_file_handle);
		this->_file_handle = nullptr;
	}
}

const int tissuestack::imaging::TissueStackImageData::getFileDescriptor()
{
	if (this->_file_handle == nullptr)
		this->openFileHandle();

	return fileno(this->_file_handle);
}

void tissuestack::imaging::TissueStackImageData::openFileHandle(bool close_open_handle)
{
	if (this->_file_handle)
	{
		if (!close_open_handle) return;
		else this->closeFileHandle();
	}

	this->_file_handle = fopen(this->_file_name.c_str(), "ro");
}

const tissuestack::imaging::TissueStackImageData * tissuestack::imaging::TissueStackImageData::fromFile(const std::string & filename)
{
	if (filename.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Image with empty file name???");

	if (!tissuestack::utils::System::fileExists(filename))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Image file does not exist!");

	// check extension (case insensitive)
	std::string fileNameAllUpperCase = filename;
	std::transform(fileNameAllUpperCase.begin(), fileNameAllUpperCase.end(), fileNameAllUpperCase.begin(), toupper);

	size_t position = fileNameAllUpperCase.rfind(".NII");
	if (position + 4 == filename.length())
		return new TissueStackNiftiData(filename);
	position = fileNameAllUpperCase.rfind(".NII.GZ");
	if (position + 7 == filename.length())
		return new TissueStackNiftiData(filename);
	position = fileNameAllUpperCase.rfind(".MNC");
	if (position + 4 == filename.length())
		return new TissueStackMincData(filename);

	// if not recognized by extension, we'll assume it's raw and if not the raw constructor will tell us otherwise
	return new tissuestack::imaging::TissueStackRawData(filename);
}

const tissuestack::imaging::TissueStackImageData * tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(
		const unsigned long long id, const bool includePlanes)
{
	const std::vector<const tissuestack::imaging::TissueStackImageData *>  ret =
			tissuestack::database::DataSetDataProvider::queryById(id, includePlanes);
	if (ret.empty()) return nullptr;

	return ret[0];
}

tissuestack::imaging::TissueStackImageData::TissueStackImageData(const std::string & filename)
	: tissuestack::imaging::TissueStackImageData::TissueStackImageData(
			filename,
			tissuestack::imaging::FORMAT::MINC) {}

tissuestack::imaging::TissueStackImageData::TissueStackImageData(
		const std::string & filename,
		tissuestack::imaging::FORMAT format) :
			_file_name(filename), _format(format) {}

void tissuestack::imaging::TissueStackImageData::initializeDimensions(const bool omitTransformationMatrix)
{
	// fallback identity matrix for missing transformation info
	const std::string identity = this->constructIdentityMatrixForDimensionNumber();

	this->setIsotropyFactors();

	// set width and height
	for (auto dim : this->_dim_order)
	{
		this->setWidthAndHeightByDimension(dim);
		if (!omitTransformationMatrix)
			this->setTransformationMatrixByDimension(dim);

		// double check if we got something for a transformation matrix
		tissuestack::imaging::TissueStackDataDimension * d =
			const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimensionByLongName(dim));
		if (d->getTransformationMatrix().empty())
			d->setTransformationMatrix(identity);
	}
}
void tissuestack::imaging::TissueStackImageData::setIsotropyFactors()
{
	// set isotropy factor which is all 1s if isotropic
	// if anisotropic, there will be at least 1 (or more) dimension
	//which will be the 'standard' to relate the others to
	float base_value = 1;

	std::vector<float> copyOfSteps = this->_steps;
	std::sort (copyOfSteps.begin(), copyOfSteps.end());

	float prvValue = -1;
	for (auto step : copyOfSteps)
	{
		if (step == prvValue)
		{
			base_value = static_cast<float>(fabs(step));
			break;
		}
		prvValue = static_cast<float>(fabs(step));
	}
	unsigned int numberOfBaseValues = 0;
	for (auto step : copyOfSteps)
		if (step == base_value)
			numberOfBaseValues++;

	if (numberOfBaseValues == copyOfSteps.size() || copyOfSteps.size() == 1)
		base_value = 1;
	else // take smallest value
		base_value = copyOfSteps[0];

	// now that we have the base value to compare to, let's compute the ratios
	for (unsigned int i=0; i < this->_steps.size(); i++)
	{
		if (base_value == 1 || this->_steps[i] == base_value)
			const_cast<tissuestack::imaging::TissueStackDataDimension *>(
				this->getDimensionByOrderIndex(i))->setIsotropyFactor(1);
		else
			const_cast<tissuestack::imaging::TissueStackDataDimension *>(
				this->getDimensionByOrderIndex(i))->setIsotropyFactor(static_cast<float>(fabs(this->_steps[i])) / base_value);
	}
}

void tissuestack::imaging::TissueStackImageData::initializeOffsetsForNonRawFiles()
{
	unsigned long long int offset = this->_header.length();
	for (auto dim : this->_dim_order)
	{
		tissuestack::imaging::TissueStackDataDimension * d =
			const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimensionByLongName(dim));
		d->setSliceSizeFromGivenWidthAndHeight();
		d->setOffSet(offset);
		offset += (d->getSliceSize() * d->getNumberOfSlices() * static_cast<unsigned long long int>(3));
	}
}

inline const std::string tissuestack::imaging::TissueStackImageData::constructIdentityMatrixForDimensionNumber() const
{
	const unsigned short numOfDimensions = this->_dim_order.size();

	if (numOfDimensions < 2) return "[[1,0],[0,1]]";

	std::ostringstream json;
	json << "[";
	for (unsigned short row=0;row<=numOfDimensions;row++)
	{
		if (row != 0) json << ",";
		json << "[";
		for (unsigned short col=0;col<=numOfDimensions;col++)
		{
			if (col != 0) json << ",";
			json << (row == col ? "1" : "0");
		}
		json << "]";
	}
	json << "]";

	return json.str();
}

inline void tissuestack::imaging::TissueStackImageData::setWidthAndHeightByDimension(const std::string & dimension)
{
	unsigned int width = 0;
	unsigned int height = 0;

	unsigned int anisotropicWidth = 0;
	unsigned int anisotropicHeight = 0;

	tissuestack::imaging::TissueStackDataDimension * widthDimension = nullptr;
	tissuestack::imaging::TissueStackDataDimension * heightDimension = nullptr;

	tissuestack::imaging::TissueStackDataDimension * presentDimension =
		const_cast<tissuestack::imaging::TissueStackDataDimension *>(
			this->getDimensionByLongName(dimension));

	if (dimension.at(0) == 'x')
	{
		if (this->getNumberOfDimensions() == 2)
		{
			widthDimension = nullptr;
			heightDimension = nullptr;
			width = presentDimension->getNumberOfSlices();
			height = this->getDimension('y')->getNumberOfSlices();
		} else
		{
			widthDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('y'));
			heightDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('z'));
		}
	} else if (dimension.at(0) == 'y')
	{
		if (this->getNumberOfDimensions() == 2)
		{
			widthDimension = nullptr;
			heightDimension = nullptr;
			width = presentDimension->getNumberOfSlices();
			height = this->getDimension('x')->getNumberOfSlices();
		} else
		{
			widthDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('x'));
			heightDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('z'));
		}
	} else if (dimension.at(0) == 'z')
	{
		widthDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('x'));
		heightDimension = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('y'));
	} else
		THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException, "Dimension cannot be matched to x,y or z!");

	if (widthDimension)
	{
		width = widthDimension->getNumberOfSlices();
		if (widthDimension->getIsotropyFactor() == 1)
			anisotropicWidth = width;
		else
			anisotropicWidth =
				static_cast<unsigned int>(
					static_cast<float>(width) * widthDimension->getIsotropyFactor());
	}
	if (heightDimension)
	{
		height = heightDimension->getNumberOfSlices();
		if (heightDimension->getIsotropyFactor() == 1)
			anisotropicHeight = height;
		else
			anisotropicHeight =
				static_cast<unsigned int>(
					static_cast<float>(height) * heightDimension->getIsotropyFactor());
	}

	presentDimension->setWidthAndHeight(
		width, height, anisotropicWidth, anisotropicHeight);
}

inline void tissuestack::imaging::TissueStackImageData::setTransformationMatrixByDimension(const std::string & dimension)
{
	tissuestack::imaging::TissueStackDataDimension * dim =
		const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimensionByLongName(dimension));

	if (dim == nullptr)
		dim->setTransformationMatrix(this->constructIdentityMatrixForDimensionNumber());

	std::ostringstream transformationMatrix;
	transformationMatrix << "[";

	std::string tmp = "";
	if (dimension.at(0) == 'x')
	{
		tmp =
			this->setTransformationMatrixByDimension0(
				0, this->getIndexForPlane('y'));
		if (tmp.empty()) return;
		transformationMatrix << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				1, this->getIndexForPlane('z'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				2, this->getIndexForPlane('x'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		transformationMatrix << "," << this->getAdjointMatrix();
	} else if (dimension.at(0) == 'y')
	{
		tmp =
			this->setTransformationMatrixByDimension0(
				0, this->getIndexForPlane('x'));
		if (tmp.empty()) return;
		transformationMatrix << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				1, this->getIndexForPlane('z'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				2, this->getIndexForPlane('y'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		transformationMatrix << "," << this->getAdjointMatrix();
	} else if (dimension.at(0) == 'z')
	{
		tmp =
			this->setTransformationMatrixByDimension0(
				0, this->getIndexForPlane('x'));
		if (tmp.empty()) return;
		transformationMatrix << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				1, this->getIndexForPlane('y'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		tmp =
			this->setTransformationMatrixByDimension0(
				2, this->getIndexForPlane('z'));
		if (tmp.empty()) return;
		transformationMatrix << "," << tmp;
		transformationMatrix << "," << this->getAdjointMatrix();
	}

	transformationMatrix << "]";
	dim->setTransformationMatrix(transformationMatrix.str());
}

inline const std::string tissuestack::imaging::TissueStackImageData::getAdjointMatrix() const
{
	const unsigned short numberOfDims = this->_dim_order.size();
	if (numberOfDims < 2)
		return "[0, 1]";

	std::ostringstream adjointMatrix;
	adjointMatrix << "[";
	for (unsigned short i=0;i<=numberOfDims;i++)
	{
		if (i !=0) adjointMatrix << ",";
		if (i == numberOfDims)
			adjointMatrix << "1";
		else
			adjointMatrix << "0";
	}
	adjointMatrix << "]";
	return adjointMatrix.str();
}

inline const std::string tissuestack::imaging::TissueStackImageData::setTransformationMatrixByDimension0(
	const short step_index,
	const short index)
{
	if (index < 0 || step_index < 0
		|| step_index >= static_cast<unsigned short>(this->_dim_order.size())
		|| index >= static_cast<unsigned short>(this->getSteps().size())
		|| index >= static_cast<unsigned short>(this->getCoordinates().size())) return "";

	std::ostringstream transformationMatrix;
	const unsigned short numberOfDimensions = this->_dim_order.size();
	transformationMatrix << "[";

	unsigned short i = 0;
	while (i<=numberOfDimensions)
	{
		if (i != 0) transformationMatrix << ",";
		if (i == step_index)
			transformationMatrix << std::to_string(this->getSteps()[index]);
		else if (i == numberOfDimensions)
			transformationMatrix << std::to_string(this->getCoordinates()[index]);
		else
			transformationMatrix << "0";
		i++;
	}
	transformationMatrix << "]";

	return transformationMatrix.str();
}

inline const short tissuestack::imaging::TissueStackImageData::getIndexForPlane(const char plane) const
{
	if (plane == ' ') return -1;

	short index = 0;
	for (auto p : this->_dim_order)
	{
		if (plane == p.at(0))
			return index;
		index++;
	}

	return -1;
}

const std::string tissuestack::imaging::TissueStackImageData::getFileName() const
{
	return this->_file_name;
}

const float tissuestack::imaging::TissueStackImageData::getResolutionMm() const
{
	return this->_resolutionMm;
}

void tissuestack::imaging::TissueStackImageData::setResolutionMm(const float resolution_mm)
{
	this->_resolutionMm = resolution_mm;
}

const tissuestack::imaging::TissueStackDataDimension * tissuestack::imaging::TissueStackImageData::getDimensionByLongName(const std::string & dimension) const
{
	if (dimension.empty()) return nullptr;

	return this->getDimension(dimension.at(0));
}

const tissuestack::imaging::TissueStackDataDimension * tissuestack::imaging::TissueStackImageData::getDimensionByOrderIndex(const unsigned short index) const
{
	if (index+1 > static_cast<unsigned short>(this->_dim_order.size()))
		return nullptr;

	return this->getDimensionByLongName(this->_dim_order[index]);
}

const tissuestack::imaging::TissueStackDataDimension * tissuestack::imaging::TissueStackImageData::getDimension(const char dimension_letter) const
{
	try
	{
		return this->_dimensions.at(dimension_letter);

	} catch (std::out_of_range & not_found) {
		return nullptr;
	}
}

const float tissuestack::imaging::TissueStackImageData::getImageDataMinumum() const
{
	return this->_global_min_value;
}

const float tissuestack::imaging::TissueStackImageData::getImageDataMaximum() const
{
	return this->_global_max_value;
}

void tissuestack::imaging::TissueStackImageData::setImageDataBounds(const float min, const float max)
{
	this->_global_min_value = min;
	this->_global_max_value = max;
}

void tissuestack::imaging::TissueStackImageData::setDataBaseId(const unsigned long long int id)
{
	this->_database_id = id;
}

const unsigned long long int tissuestack::imaging::TissueStackImageData::getDataBaseId() const
{
	return this->_database_id;
}

void tissuestack::imaging::TissueStackImageData::setFormat(int original_format)
{
	switch (original_format)
	{
		case tissuestack::imaging::FORMAT::MINC:
			this->_format = tissuestack::imaging::FORMAT::MINC;
			break;
		case tissuestack::imaging::FORMAT::NIFTI:
			this->_format = tissuestack::imaging::FORMAT::NIFTI;
			break;
		case tissuestack::imaging::FORMAT::RAW:
			this->_format = tissuestack::imaging::FORMAT::RAW;
			break;
		default:
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Incompatible Original Format!");
			break;
	}
}

const tissuestack::imaging::FORMAT tissuestack::imaging::TissueStackImageData::getFormat() const
{
	return this->_format;
}

const bool tissuestack::imaging::TissueStackImageData::containsAssociatedDataSet(unsigned long long int dataset_id) const
{
	for (auto a_ds : this->_associated_data_sets)
		if (a_ds->getDataBaseId() == dataset_id)
			return true;

	return false;
}

const bool tissuestack::imaging::TissueStackImageData::hasZeroDimensions() const
{
	return this->_dim_order.empty();
}

const bool tissuestack::imaging::TissueStackImageData::hasNoAssociatedDataSets() const
{
	return this->_associated_data_sets.empty();
}

void tissuestack::imaging::TissueStackImageData::clearAssociatedDataSets()
{
	this->_associated_data_sets.clear();
}

const std::string tissuestack::imaging::TissueStackImageData::getZoomLevelsAsJson() const
{
	if (this->_zoom_levels.empty()) // return default
		return "[0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2.00, 2.25, 2.5]";

	std::ostringstream json;
	json << "[";
	int i=0;
	for (auto z : this->_zoom_levels)
	{
		if (i != 0) json << ",";
		json << std::to_string(z);
		i++;
	}
	json << "]";

	return json.str();
}

const std::string tissuestack::imaging::TissueStackImageData::toJson(
		const bool includePlanes,
		const bool dontDescendIntoAssociated) const
{
	std::ostringstream json;

	json << "{ \"id\": " << std::to_string(this->_database_id);
	json << ", \"filename\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_file_name) << "\"";
	if (!this->_description.empty())
		json << ", \"description\": \"" << this->_description << "\"";
	if (this->_lookup)
		json << ", \"lookupValues\": " << this->_lookup->toJson();

	// data set dimensions
	if (includePlanes && !this->_dim_order.empty())
	{
		json << ", \"planes\": [ ";
		int j=0;
		for (auto p : this->_dim_order)
		{
			const tissuestack::imaging::TissueStackDataDimension * dim =
				this->getDimensionByLongName(p);
			if (j != 0) json << ",";
			json << "{ \"name\": \"" << dim->getName()[0] << "\"";
			json << ", \"maxSlices\": " << std::to_string(dim->getNumberOfSlices()-1);
			json << ", \"maxX\": " << std::to_string(dim->getAnisotropicWidth());
			json << ", \"maxY\": " << std::to_string(dim->getAnisotropicHeight());
			json << ", \"origX\": " << std::to_string(dim->getWidth());
			json << ", \"origY\": " << std::to_string(dim->getHeight());
			json << ", \"isTiled\": " << (this->_is_tiled ? "true" : "false");
			json << ", \"oneToOneZoomLevel\": " << std::to_string(this->_one_to_one_zoom_level);
			json << ", \"resolutionMm\": " << std::to_string(this->getResolutionMm());
			json << ", \"transformationMatrix\": \"" << dim->getTransformationMatrix() << "\"";
			json << ", \"zoomLevels\": \"" << this->getZoomLevelsAsJson() << "\"";
			json << ", \"valueRangeMin\": \"" << std::to_string(this->getImageDataMinumum()) << "\"";
			json << ", \"valueRangeMax\": \"" << std::to_string(this->getImageDataMaximum()) << "\"";
			json << "}";
			j++;
		}
		json << "]";
	}

	// associated data sets
	if (includePlanes && !dontDescendIntoAssociated)
	{
		json << ", \"associatedDataSets\": [";

		if (!this->_associated_data_sets.empty())
		{
			int k=0;
			for (auto a_ds : this->_associated_data_sets)
			{
				if (k != 0)
					json << ",";
				json << "{ \"associatedDataSet\": ";
				json << a_ds->toJson(
						false,
						a_ds->containsAssociatedDataSet(this->getDataBaseId()));
				json << ", \"datasetId\": ";
				json << std::to_string(this->getDataBaseId());
				json << "}";
				k++;
			}
		}
		json << "]";
	}

	json << "}";

	return json.str();
}

const std::unordered_map<char, const tissuestack::imaging::TissueStackDataDimension *> tissuestack::imaging::TissueStackImageData::getDimensionMap() const
{
	return this->_dimensions;
}

void tissuestack::imaging::TissueStackImageData::addDimension(tissuestack::imaging::TissueStackDataDimension * dimension)
{
	if (dimension == nullptr) return;

	if (this->getDimensionByLongName(dimension->getName()))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"2 dimensions with same starting letter cannot be added twice. Just a convention for technical convenience, sorry..");

	this->_dim_order.push_back(dimension->getName());
	this->_dimensions[dimension->getName().at(0)] = dimension;
}

void tissuestack::imaging::TissueStackImageData::addCoordinate(float coord)
{
	return this->_coordinates.push_back(coord);
}

void tissuestack::imaging::TissueStackImageData::addStep(float step)
{
	return this->_steps.push_back(step);
}

const std::vector<float> tissuestack::imaging::TissueStackImageData::getCoordinates() const
{
	return this->_coordinates;
}

const std::vector<float> tissuestack::imaging::TissueStackImageData::getSteps() const
{
	return this->_steps;
}

void tissuestack::imaging::TissueStackImageData::dumpDataDimensionInfoIntoDebugLog() const
{
	for (const std::string dim : this->_dim_order)
		this->getDimensionByLongName(dim)->dumpDataDimensionInfoIntoDebugLog();
}

void tissuestack::imaging::TissueStackImageData::addAssociatedDataSet(
	const tissuestack::imaging::TissueStackImageData * associatedDataSet)
{
	if (associatedDataSet)
		this->_associated_data_sets.push_back(associatedDataSet);
}

void tissuestack::imaging::TissueStackImageData::setMembersFromDataBaseInformation(
		const unsigned long long int id,
		const std::string description,
		const bool is_tiled,
		const std::vector<float> zoom_levels,
		const unsigned short one_to_one_zoom_level,
		const float resolution_in_mm,
		const float global_min_value,
		const float global_max_value,
		const tissuestack::imaging::TissueStackLabelLookup * lookup)
{
	this->_database_id = id,
	this->_description = description;
	this->_is_tiled = is_tiled;
	if (!zoom_levels.empty())
		this->_zoom_levels = zoom_levels;
	this->_one_to_one_zoom_level = one_to_one_zoom_level;
	this->_resolutionMm = resolution_in_mm;
	this->_global_min_value = global_min_value;
	this->_global_max_value = global_max_value;
	this->_lookup = lookup;
}

const unsigned short tissuestack::imaging::TissueStackImageData::getNumberOfDimensions() const
{
	return this->_dim_order.size();
}

const std::string tissuestack::imaging::TissueStackImageData::getDescription() const
{
	return this->_description;
}

void tissuestack::imaging::TissueStackImageData::setDescription(const std::string description)
{
	this->_description = description;
}

const bool tissuestack::imaging::TissueStackImageData::isTiled() const
{
	return this->_is_tiled;
}

const std::vector<float> tissuestack::imaging::TissueStackImageData::getZoomLevels() const
{
	return this->_zoom_levels;
}

const unsigned short tissuestack::imaging::TissueStackImageData::getOneToOneZoomLevel() const
{
	return this->_one_to_one_zoom_level;
}

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackImageData::getLookup() const
{
	return this->_lookup;
}

void tissuestack::imaging::TissueStackImageData::dumpImageDataIntoDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug("Image Data For File:%s\n", this->_file_name.c_str());
	tissuestack::logging::TissueStackLogger::instance()->debug("Original File Format: %u\n", this->_format);
	this->dumpDataDimensionInfoIntoDebugLog();

	std::ostringstream in;

	in << "Coordinates: ";
	for (auto v : this->_coordinates)
		in << v << "\t";
	tissuestack::logging::TissueStackLogger::instance()->debug("%s\n", in.str().c_str());

	in.str("");
	in.clear();

	in << "Steps: ";
	for (auto v : this->_steps)
		in << v << "\t";
	tissuestack::logging::TissueStackLogger::instance()->debug("%s\n", in.str().c_str());

	if (!this->isRaw()) return;

	in.str("");
	in.clear();

	const tissuestack::imaging::TissueStackRawData * __this = static_cast<const tissuestack::imaging::TissueStackRawData *>(this);
	tissuestack::logging::TissueStackLogger::instance()->debug("Raw Version: %u\n", __this->_raw_version);
	tissuestack::logging::TissueStackLogger::instance()->debug("Raw Type: %u\n", __this->_raw_type);
}

const std::vector<std::string> tissuestack::imaging::TissueStackImageData::getDimensionOrder() const
{
	return this->_dim_order;
}

void tissuestack::imaging::TissueStackImageData::setHeader(const std::string header)
{
	this->_header = header;
}

const std::string tissuestack::imaging::TissueStackImageData::getHeader() const
{
	return this->_header;
}

void tissuestack::imaging::TissueStackImageData::generateRawHeader()
{
	const std::string headerBeginning =
		std::string("@IaMraW@V") +
		std::to_string(tissuestack::imaging::RAW_FILE_VERSION::V1) +
		"|";

	if (this->_dimensions.empty())
		THROW_TS_EXCEPTION(
			tissuestack::common::TissueStackApplicationException,
			"Cannot generate raw header for 0 dimension image data!");

	std::ostringstream header;

	unsigned short i =0;
	for (i=0; i < this->_dim_order.size();i++) // slice numbers
	{
		header << std::to_string(this->getDimensionByOrderIndex(i)->getNumberOfSlices());
		if (i < this->_dim_order.size()-1) header << ":";
	}
	header << "|";
	i =0;
	for (auto coord : this->_coordinates) // coords
	{
		header << std::to_string(coord);
		if (i < this->_coordinates.size()-1) header << ":";
		i++;
	}
	header << "|";
	i =0;
	for (auto step : this->_steps) // steps
	{
		header << std::to_string(step);
		if (i < this->_steps.size()-1) header << ":";
		i++;
	}
	header << "|";
	i =0;
	for (auto name : this->_dim_order) // dimension names
	{
		header << name;
		if (i < this->_dim_order.size()-1) header << ":";
		i++;
	}
	header << "|"; // finish off with original format
	header << std::to_string(this->_format);
	header << "|";

	const std::string headerString  = header.str();

	this->_header = headerBeginning +
		std::to_string(headerString.length()) +
		"|" + headerString;
}
