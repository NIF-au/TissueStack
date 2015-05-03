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
#include "services.h"

tissuestack::services::TissueStackTilingTask::TissueStackTilingTask(
	const std::string id,
	const std::string input_file,
	const std::string tile_dir,
	const std::vector<std::string> dimensions,
	const std::vector<unsigned short> zoom_levels,
	const std::string color_map,
	const unsigned short square,
	const std::string image_format) :
	tissuestack::services::TissueStackTask(id, input_file)
{
	if (!this->isInputDataRaw())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Tiling needs raw format data");

	// check tile directory
	// with simultaneous threads having a go at this, we can have it that mkdir fails
	// because another thread has already done it
	unsigned short tries = 5;
	while (true)
	{
		if (tissuestack::utils::System::directoryExists(tile_dir))
			break;

		if (tries <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create tile directory!");

		if (!tissuestack::utils::System::createDirectory(tile_dir, 0755))
			tries--;
		usleep(100 * 1000); // wait 100 millis
	}
	this->_tile_dir = tile_dir;

	// check color map
	if (color_map.compare("grey") != 0
			&& color_map.compare("gray") != 0
			&& tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist()
			&& tissuestack::imaging::TissueStackColorMapStore::instance()->findColorMap(color_map) == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not find colormap!");
	this->_color_map = color_map;

	// check square length
	if (square == 0 || square > 1024)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Square length should be in between 0 and 1024!");
	this->_square = square;

	// check image format
	std::string pngOrJpeg = image_format;
	std::transform(pngOrJpeg.begin(), pngOrJpeg.end(), pngOrJpeg.begin(), tolower);
	if (pngOrJpeg.compare("png") != 0
			&& pngOrJpeg.compare("jpg") != 0
			&& pngOrJpeg.compare("jpeg") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image format can only be png or jpg, png being preferred!");
	this->_image_format = image_format;

	// check dimensions
	if (dimensions.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We habe to have at least one dimension for tiling!");
	if (this->getInputImageData()->get2DDimension() == nullptr)
	{
		for (auto d : dimensions)
			if (this->getInputImageData()->getDimensionByLongName(d) == nullptr
				&& this->getInputImageData()->getDimension(d.at(0)) == nullptr)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Data Set to be tiled does not have requested dimension");
		this->_dimensions = dimensions;
	} else // 2D DATA
		this->_dimensions.push_back(this->getInputImageData()->get2DDimension()->getName());

	//check the zoom levels
	if (zoom_levels.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We habe to have at least one zoom level for tiling!");
	for (auto z : zoom_levels)
		if (z > this->getInputImageData()->getZoomLevels().size())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Requested zoom level exceeds zoom factor size!");
	this->_zoom_levels = zoom_levels;

	// set total number of slices we have to work off
	unsigned long long int totalSlices = 0;
	if (this->getInputImageData()->get2DDimension() == nullptr)
		for (auto d : dimensions)
			totalSlices += this->getInputImageData()->getDimension(d.at(0))->getNumberOfSlices();
	else
		totalSlices = this->getInputImageData()->get2DDimension()->getNumberOfSlices();
	this->setTotalSlices(totalSlices * static_cast<unsigned long long int>(this->_zoom_levels.size()));
}

tissuestack::services::TissueStackTilingTask::~TissueStackTilingTask() {}

const std::string tissuestack::services::TissueStackTilingTask::getImageFormat() const
{
	return this->_image_format;
}

const std::string tissuestack::services::TissueStackTilingTask::getColorMap() const
{
	return this->_color_map;
}

const std::string tissuestack::services::TissueStackTilingTask::getTileDir() const
{
	return this->_tile_dir;
}

const unsigned int tissuestack::services::TissueStackTilingTask::getSquareLength() const
{
	return this->_square;
}

const std::vector<unsigned short> tissuestack::services::TissueStackTilingTask::getZoomLevels() const
{
	return this->_zoom_levels;
}

const std::vector<std::string> tissuestack::services::TissueStackTilingTask::getDimensions() const
{
	return this->_dimensions;
}

const bool tissuestack::services::TissueStackTilingTask::includesZoomLevel(
		const tissuestack::services::TissueStackTilingTask * anotherTask) const
{
	if (anotherTask == nullptr) return false;

	const std::vector<unsigned short> anotherTasksZoomLevels =
		anotherTask->getZoomLevels();

	for (auto another_z : anotherTasksZoomLevels)
		for (auto z : this->_zoom_levels)
			if (z == another_z)
				return true;

	return false;
}

const tissuestack::services::TissueStackTaskType tissuestack::services::TissueStackTilingTask::getType() const
{
	return tissuestack::services::TissueStackTaskType::TILING;
}

const std::string tissuestack::services::TissueStackTilingTask::getParametersForTaskFile() const
{
	std::ostringstream paramString;

	paramString << this->_tile_dir << ":";

	int j=0;
	for (auto d : this->_dimensions)
	{
		if (j != 0)
			paramString << ",";
		paramString << d;
		j++;
	}

	paramString << ":";
	j=0;
	for (auto z : this->_zoom_levels)
	{
		if (j != 0)
			paramString << ",";
		paramString << std::to_string(z);
		j++;
	}

	paramString << ":" << this->_color_map << ":";
	paramString << std::to_string(this->_square) << ":";
	paramString << this->_image_format;

	return paramString.str();
}

void tissuestack::services::TissueStackTilingTask::dumpTaskToDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug(
			"Tiling Task: %s => %s\n",
			this->getInputImageData()->getFileName().c_str(), this->getParametersForTaskFile().c_str());
}
