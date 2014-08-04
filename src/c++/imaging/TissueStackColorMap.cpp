#include "imaging.h"

tissuestack::imaging::TissueStackColorMap::TissueStackColorMap(const std::string & filename) : _colormap_id(filename)
{
	// TODO: map continuous color maps


	tissuestack::logging::TissueStackLogger::instance()->info("Loading color map file %s\n", filename.c_str());

	std::string line = "";
	std::ifstream file_stream;

	try
	{
		file_stream.open(filename);

		this->preFillColorMapArray();

		for( std::string line; std::getline( file_stream, line ); )
		{
			if (line.empty()) continue;

			const std::vector<std::string> tokens = tissuestack::utils::Misc::tokenizeString(line, ' ');
			if (tokens.size() < 4)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Color Map file has to have 4 tokens!");

			int colNr = 0;
			unsigned short gray;
			unsigned short red;
			unsigned short green;
			unsigned short blue;
			unsigned long long int value;

			for (std::string s : tokens)
			{
				if (s.empty()) continue;

				switch (colNr)
				{
					case 0:	// the gray value
						value = strtouq(s.c_str(), NULL, 10);
						if (value > 255)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has gray value of higher than 255!");
						gray = static_cast<unsigned short>(value);
						break;
					case 1: // Red
						value = strtouq(s.c_str(), NULL, 10);
						if (value > 255)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has red value of higher than 255!");
						red = static_cast<unsigned short>(value);
						break;
					case 2: // Green
						value = strtouq(s.c_str(), NULL, 10);
						if (value > 255)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has green value of higher than 255!");
						green = static_cast<unsigned short>(value);
						break;
					case 3: // Blue
						value = strtouq(s.c_str(), NULL, 10);
						if (value > 255)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
									"Lookup file has blue value of higher than 255!");
						blue = static_cast<unsigned short>(value);
						break;
					default:
						break;
				}
				colNr++;
			}
			if (colNr != 4)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Color Map file has to have 4 tokens!");

			this->_gray_indexed_rgb_mapping[gray][0] = red;
			this->_gray_indexed_rgb_mapping[gray][1] = green;
			this->_gray_indexed_rgb_mapping[gray][2] = blue;
		}
		file_stream.close();

		tissuestack::logging::TissueStackLogger::instance()->info("Finished Loading color map file.\n");
	}	catch (tissuestack::common::TissueStackException & ex) {
		tissuestack::logging::TissueStackLogger::instance()->debug("%s\n", ex.what());
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
	tissuestack::logging::TissueStackLogger::instance()->info("Copied color map file from lookup file: %s\n",
			label_lookup_file->getLabelLookupId().c_str());
}

void tissuestack::imaging::TissueStackColorMap::preFillColorMapArray()
{
	unsigned short i=0;
	for (auto & rgb : this->_gray_indexed_rgb_mapping)
	{
		rgb[0] = rgb[1] = rgb[2] = i;
		i++;
	}
}

const std::array<const unsigned short, 3> tissuestack::imaging::TissueStackColorMap::getRGBMapForGrayValue(const unsigned short & gray) const
{
	const std::array<const unsigned short, 3> ret =
			{
				this->_gray_indexed_rgb_mapping[gray][0],
				this->_gray_indexed_rgb_mapping[gray][1],
				this->_gray_indexed_rgb_mapping[gray][2],
			};
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

const std::string tissuestack::imaging::TissueStackColorMap::getColorMapId() const
{
	return this->_colormap_id;
}

void tissuestack::imaging::TissueStackColorMap::dumpColorMapToDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug("Dumping Color Map: %s\n", this->getColorMapId().c_str());

	for (auto rgb : this->_gray_indexed_rgb_mapping)
	{
		tissuestack::logging::TissueStackLogger::instance()->debug("%u\t%u\t%u\n", rgb[0], rgb[1], rgb[2]);
	}
}
