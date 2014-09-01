#include "imaging.h"
#include "database.h"

tissuestack::imaging::TissueStackImageData::TissueStackImageData(
		const unsigned long long int id, const std::string filename) :
	_file_name(filename), _format(tissuestack::imaging::FORMAT::DATABASE), _database_id(id), _is_tiled(false), _resolution_in_mm(0) {}

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

	// check extension
	size_t position = filename.rfind(".nii");
	if (position + 4 == filename.length())
		return new TissueStackNiftiData(filename);
	position = filename.rfind(".nii.gz");
	if (position + 7 == filename.length())
		return new TissueStackNiftiData(filename);
	position = filename.rfind(".mnc");
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

void tissuestack::imaging::TissueStackImageData::initializeDimensions()
{
	for (auto dim : this->_dim_order)
	{
		this->setWidthAndHeightByDimension(dim);
		this->setTransformationMatrixByDimension(dim);
	}
}

void tissuestack::imaging::TissueStackImageData::setWidthAndHeightByDimension(const std::string & dimension)
{
	std::array<unsigned int,2> widthAndHeight = {{0, 0}};

	tissuestack::imaging::TissueStackDataDimension * width = nullptr;
	tissuestack::imaging::TissueStackDataDimension * height = nullptr;

	if (dimension.at(0) == 'x')
	{
		width = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('y'));
		height = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('z'));
	} else if (dimension.at(0) == 'y')
	{
		width = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('x'));
		height = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('z'));
	} else if (dimension.at(0) == 'z')
	{
		width = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('x'));
		height = const_cast<tissuestack::imaging::TissueStackDataDimension *>(this->getDimension('y'));
	}

	if (width) widthAndHeight[0] = width->getNumberOfSlices();
	if (height) widthAndHeight[1] = height->getNumberOfSlices();

	const_cast<tissuestack::imaging::TissueStackDataDimension *>(
			this->getDimensionByLongName(dimension))->setWidthAndHeight(
					const_cast<const std::array<unsigned int,2> & >(widthAndHeight));
}

void tissuestack::imaging::TissueStackImageData::setTransformationMatrixByDimension(const std::string & dimension)
{
	// TODO: implement
	std::string transformationMatrix = "";

	const_cast<tissuestack::imaging::TissueStackDataDimension *>(
			this->getDimensionByLongName(dimension))->setTransformationMatrix(transformationMatrix);
}

const std::string tissuestack::imaging::TissueStackImageData::getFileName() const
{
	return this->_file_name;
}

const tissuestack::imaging::TissueStackDataDimension * tissuestack::imaging::TissueStackImageData::getDimensionByLongName(const std::string & dimension) const
{
	if (dimension.empty()) return nullptr;

	return this->getDimension(dimension.at(0));
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

const unsigned short tissuestack::imaging::TissueStackImageData::getImageDataMinumum() const
{
	return this->_global_min_value;
}

const unsigned short tissuestack::imaging::TissueStackImageData::getImageDataMaximum() const
{
	return this->_global_max_value;
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
			json << ", \"maxSlices\": " << std::to_string(dim->getNumberOfSlices());
			json << ", \"maxX\": " << std::to_string(dim->getWidth());
			json << ", \"maxY\": " << std::to_string(dim->getHeight());
			json << ", \"isTiled\": " << (this->_is_tiled ? "true" : "false");
			json << ", \"oneToOneZoomLevel\": " << std::to_string(this->_one_to_one_zoom_level);
			json << ", \"resolutionMm\": " << std::to_string(this->_resolution_in_mm);
			json << ", \"transformationMatrix\": \"" << dim->getTransformationMatrix() << "\"";
			json << ", \"zoomLevels\": \"" << this->getZoomLevelsAsJson() << "\"";
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
		const tissuestack::imaging::TissueStackLabelLookup * lookup)
{
	this->_database_id = id,
	this->_description = description;
	this->_is_tiled = is_tiled;
	if (!zoom_levels.empty())
		this->_zoom_levels = zoom_levels;
	this->_one_to_one_zoom_level = one_to_one_zoom_level;
	this->_resolution_in_mm = resolution_in_mm;
	this->_lookup = lookup;
}


const std::string tissuestack::imaging::TissueStackImageData::getDescription() const
{
	return this->_description;
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

const float tissuestack::imaging::TissueStackImageData::getResolutionInMm() const
{
	return this->_resolution_in_mm;
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
