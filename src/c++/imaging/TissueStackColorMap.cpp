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

tissuestack::imaging::TissueStackColorMap::TissueStackColorMap(const std::string & filename) : _colormap_id(filename)
{
	this->updateColorMap(filename);
}

void tissuestack::imaging::TissueStackColorMap::updateColorMap(const std::string & filename)
{
	if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
		tissuestack::logging::TissueStackLogger::instance()->info("Loading color map file %s\n", filename.c_str());

	std::string line = "";
	std::ifstream file_stream;

	try
	{
		file_stream.open(filename);

		tissuestack::imaging::TissueStackColorMap::preFillColorMapArray(this->_gray_indexed_rgb_mapping);

		std::vector<std::array<float, 4> > colorMapRanges;

		for( std::string line; std::getline( file_stream, line ); )
		{
			if (line.empty()) continue;

			const std::vector<std::string> tokens = tissuestack::utils::Misc::tokenizeString(line, ' ');
			if (tokens.size() < 4)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Color Map file has to have 4 tokens!");

			int colNr = 0;
			float gray = -1;
			float red = -1;
			float green = -1;
			float blue = -1;

			for (std::string s : tokens)
			{
				if (s.empty()) continue;

				switch (colNr)
				{
					case 0:	// the gray value
						gray = strtof(s.c_str(), NULL);
						if (gray < 0 || gray > 1.0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has gray range outside of 0 and 1!");
						break;
					case 1: // Red
						red = strtof(s.c_str(), NULL);
						if (red < 0 || red > 1.0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has red range outside of 0 and 1!");
						break;
					case 2: // Green
						green = strtof(s.c_str(), NULL);
						if (green < 0 || green > 1.0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has green range outside of 0 and 1!");
						break;
					case 3: // Blue
						blue = strtof(s.c_str(), NULL);
						if (blue < 0 || blue > 1.0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has blue range outside of 0 and 1!");
						break;
					default:
						break;
				}
				if (colNr > 3) break; // we ignore further columns

				colNr++;
			}
			if (colNr < 3)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Color Map file has to have at least 4 tokens!");

			colorMapRanges.push_back({{gray, red, green, blue}});
		}
		file_stream.close();

		// now go about to convert range to discrete 0-255 value mapping
		short valueRangeRow = 1;
		float offsetRGB [3] = { colorMapRanges[0][1], colorMapRanges[0][2], colorMapRanges[0][3] };
		float valueRangeStart = colorMapRanges[valueRangeRow - 1][0] * 255;
		float valueRangeEnd = colorMapRanges[valueRangeRow][0] * 255;
		unsigned short rgb = 1;
		float valueRangeDelta = 0;

		for (unsigned short index = 0; index < 256; index++) {
			// continuous to discrete mapping
			// first check if we are within desired range up the present end,
			// if not => increment row index to move on to next range
			if (index > valueRangeEnd) {
				valueRangeRow++;
				valueRangeStart = valueRangeEnd;
				valueRangeEnd = colorMapRanges[valueRangeRow][0] * 255;
				offsetRGB[0] = colorMapRanges[valueRangeRow - 1][1];
				offsetRGB[1] = colorMapRanges[valueRangeRow - 1][2];
				offsetRGB[2] = colorMapRanges[valueRangeRow - 1][3];
			}
			valueRangeDelta = valueRangeEnd - valueRangeStart;

			// iterate over RGB channels
			for (rgb = 1; rgb < 4; rgb++)
			{
				float rgbRangeStart = colorMapRanges[valueRangeRow - 1][rgb];
				float rgbRangeEnd = colorMapRanges[valueRangeRow][rgb];
				float rgbRangeDelta = rgbRangeEnd - rgbRangeStart;
				float rangeRemainder = fmodf((float) index, valueRangeDelta);

				if (rangeRemainder == 0 && rgbRangeDelta != 0 && rgbRangeDelta != 1
						&& valueRangeEnd == 255) {
					offsetRGB[rgb - 1] += rgbRangeDelta;
				} else if (rangeRemainder == 0 && rgbRangeDelta != 0 && valueRangeEnd == 255 && index == 255) {
					offsetRGB[rgb - 1] += rgbRangeDelta;
				}

				float rangeRatio =
						(rgbRangeDelta * rangeRemainder / valueRangeDelta)
								+ offsetRGB[rgb - 1];

				this->_gray_indexed_rgb_mapping[index][rgb-1] = round(rangeRatio * 255);
			}
		}

		this->marshallColorMapContentsIntoJson(colorMapRanges);

		if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
			tissuestack::logging::TissueStackLogger::instance()->info("Finished Loading color map file.\n");
	}	catch (tissuestack::common::TissueStackException & ex) {
		if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
			tissuestack::logging::TissueStackLogger::instance()->error("%s\n", ex.what());
		file_stream.close();
		throw ex;
	}	catch (std::exception & ex) {
		file_stream.close();
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not read/parse label lookup file!");
	}
}

tissuestack::imaging::TissueStackColorMap::TissueStackColorMap(const tissuestack::imaging::TissueStackLabelLookup * label_lookup_file) :
		_colormap_id(label_lookup_file->getLabelLookupId())
{
	label_lookup_file->copyGrayIndexedRgbMapping(this->_gray_indexed_rgb_mapping);
	this->marshallLookupFileContentsIntoJson();
	if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
		tissuestack::logging::TissueStackLogger::instance()->info("Copied color map file from lookup file: %s\n",
			label_lookup_file->getLabelLookupId().c_str());
}

const bool tissuestack::imaging::TissueStackColorMap::isBeingUpdated() const
{
	return this->_is_being_Updated;
}

void tissuestack::imaging::TissueStackColorMap::setUpdateFlag(const bool is_being_Updated)
{
	this->_is_being_Updated = is_being_Updated;
}

void tissuestack::imaging::TissueStackColorMap::preFillColorMapArray(std::array<unsigned short[3], 256> & color_map_array)
{
	unsigned short i=0;
	for (auto & rgb : color_map_array)
	{
		rgb[0] = rgb[1] = rgb[2] = i;
		i++;
	}
}

const std::array<const unsigned short, 3> tissuestack::imaging::TissueStackColorMap::getRGBMapForGrayValue(const unsigned short & gray) const
{
	while (this->isBeingUpdated())
		usleep(10000); // 10,000 micro seconds /10 milli seconds

	const std::array<const unsigned short, 3> ret =
			{{
				this->_gray_indexed_rgb_mapping[gray][0],
				this->_gray_indexed_rgb_mapping[gray][1],
				this->_gray_indexed_rgb_mapping[gray][2],
			}};
	return ret;
}

const tissuestack::imaging::TissueStackColorMap * tissuestack::imaging::TissueStackColorMap::fromFile(const std::string & filename)
{
	return new tissuestack::imaging::TissueStackColorMap(filename);
}

const tissuestack::imaging::TissueStackColorMap * tissuestack::imaging::TissueStackColorMap::fromLabelLookup(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	if (labelLookup == nullptr) return nullptr;

	return new tissuestack::imaging::TissueStackColorMap(labelLookup);
}

const time_t tissuestack::imaging::TissueStackColorMap::getLastModified() const
{
	return this->_last_Modification;
}

void tissuestack::imaging::TissueStackColorMap::setLastModified(const time_t lastModified)
{
	this->_last_Modification = lastModified;
}

void tissuestack::imaging::TissueStackColorMap::marshallColorMapContentsIntoJson(
		const std::vector<std::array<float, 4> > & colorMapRanges)
{
	std::ostringstream json;
	json << "\"" << this->getColorMapId() << "\": [";
	unsigned short i = 0;
	for (auto row : colorMapRanges)
	{
		if (i != 0)
			json << ",";

		json << "[" << row[0] << "," << row[1] << ","
			<< row[2] << "," << row[3] << "]";
		i++;
	}
	json << "]";

	this->_colorMapFileContentAsJson = json.str();
}
void tissuestack::imaging::TissueStackColorMap::marshallLookupFileContentsIntoJson()
{
	std::ostringstream json;
	json << "\"" << this->getColorMapId() << "\": [";
	unsigned short i = 0;
	for (auto row : this->_gray_indexed_rgb_mapping)
	{
		if (i != 0)
			json << ",";
		json << "[\"L\"," << i << "," << row[0] << ","
			<< row[1] << "," << row[2] << "]";
		i++;
	}
	json << "]";

	this->_colorMapFileContentAsJson = json.str();
}

const std::string tissuestack::imaging::TissueStackColorMap::getColorMapId() const
{
	const std::string dir = tissuestack::imaging::TissueStackColorMapStore::getColorMapDirectory();
	if (this->_colormap_id.find(dir) == 0)
		return this->_colormap_id.substr(dir.length() + 1);

	return this->_colormap_id;
}

const std::string tissuestack::imaging::TissueStackColorMap::toJson(bool originalColorMapContents) const
{
	while (this->isBeingUpdated())
		usleep(10000); // 10,000 micro seconds /10 milli seconds

	if (originalColorMapContents)
		return this->_colorMapFileContentAsJson;

	std::ostringstream json;
	json << "\"" << this->getColorMapId() << "\": [";
	unsigned short i = 0;
	for (auto row : this->_gray_indexed_rgb_mapping)
	{
		if (i != 0)
			json << ",";
		json << "[" << row[0] << "," << row[1] << ","
			<< row[2]  << "]";
		i++;
	}
	json << "]";

	return json.str();
}

void tissuestack::imaging::TissueStackColorMap::dumpColorMapToDebugLog() const
{
	if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
		tissuestack::logging::TissueStackLogger::instance()->debug("Dumping Color Map: %s\n", this->getColorMapId().c_str());

	if (this->isBeingUpdated())
		tissuestack::logging::TissueStackLogger::instance()->debug("Color Map is being updated at the moment!");

	for (auto rgb : this->_gray_indexed_rgb_mapping)
	{
		if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
			tissuestack::logging::TissueStackLogger::instance()->debug("%u\t%u\t%u\n", rgb[0], rgb[1], rgb[2]);
	}
}
