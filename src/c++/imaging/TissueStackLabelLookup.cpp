#include "imaging.h"

tissuestack::imaging::TissueStackLabelLookup::TissueStackLabelLookup(const std::string & filename) : _labellookup_id(filename)
{
	tissuestack::logging::TissueStackLogger::instance()->info("Loading label lookup file %s\n", filename.c_str());

	std::string line = "";
	std::ifstream file_stream;

	try
	{
		file_stream.open(filename);

		tissuestack::imaging::TissueStackColorMap::preFillColorMapArray(this->_gray_indexed_rgb_mapping);

		for( std::string line; std::getline( file_stream, line ); )
		{
			if (line.empty()) continue;

			const std::vector<std::string> tokens = tissuestack::utils::Misc::tokenizeString(line, '\t');
			if (tokens.size() < 5)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Lookup file has to have 5 tokens!");

			int colNr = 0;
			int gray = -1;
			unsigned short red;
			unsigned short green;
			unsigned short blue;
			unsigned long long int value;
			std::string label = "";

			for (std::string s : tokens)
			{
				if (colNr == 0) // reset
					gray = -1;

				if (colNr != 4 && s.empty()) continue;

				switch (colNr)
				{
					case 0:	// the 'potential' gray value (if lookup is ALSO used for 256 gray to rgb mapping)
						value = strtouq(s.c_str(), NULL, 10);
						if (value <= 255) gray = value;
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
					case 4: // Label
						label = s;
						break;
					default:
						break;
				}
				if (colNr > 4) break; // we ignore furher columns

				colNr++;
			}
			if (colNr < 4)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Lookup file has to have at least 5 tokens!");

			// add to rgb mapping if gray is not -1
			if (gray >=0)
			{
				this->_gray_indexed_rgb_mapping[gray][0] = red;
				this->_gray_indexed_rgb_mapping[gray][1] = green;
				this->_gray_indexed_rgb_mapping[gray][2] = blue;
			}

			// add lookup entry
			if (!label.empty())
			{
				std::ostringstream in;
				in << red << "/" << green << "/" + blue;
				std::string rgbTripleKey = in.str();
				this->_label_lookups[rgbTripleKey] = label;
			}
		}
		file_stream.close();

		tissuestack::logging::TissueStackLogger::instance()->info("Finished Loading label lookup file.\n");
	}	catch (tissuestack::common::TissueStackException & ex) {
		file_stream.close();
		throw ex;
	}	catch (std::exception & ex) {
		file_stream.close();
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not read/parse label lookup file!");
	}
}

const std::string tissuestack::imaging::TissueStackLabelLookup::getLabel(const unsigned short & red, const unsigned short & green, const unsigned short & blue) const
{
	std::ostringstream in;
	in << red << "/" << green << "/" + blue;
	std::string rgbTripleKey = in.str();

	try
	{
		return this->_label_lookups.at(rgbTripleKey);

	} catch (std::out_of_range & not_found) {
		return std::string("");
	}
}

void tissuestack::imaging::TissueStackLabelLookup::copyGrayIndexedRgbMapping(std::array<unsigned short[3], 256> & grayIndexedRgbMapping) const
{
		grayIndexedRgbMapping = this->_gray_indexed_rgb_mapping;
}

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackLabelLookup::fromFile(const std::string & filename)
{
	if (!tissuestack::utils::System::fileExists(filename))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Label Lookup File does not exist!");

	return new tissuestack::imaging::TissueStackLabelLookup(filename);
}

const std::string tissuestack::imaging::TissueStackLabelLookup::getLabelLookupId() const
{
	return this->_labellookup_id.substr(strlen(LABEL_LOOKUP_PATH) + 1);
}

void tissuestack::imaging::TissueStackLabelLookup::dumpLabelLookupToDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug("Dumping Label Lookup: %s\n", this->getLabelLookupId().c_str());

	for (auto rgb : this->_gray_indexed_rgb_mapping)
	{
		const std::string label = this->getLabel(rgb[0], rgb[1], rgb[2]);
		tissuestack::logging::TissueStackLogger::instance()->debug("%u\t%u\t%u\t%s\n", rgb[0], rgb[1], rgb[2], label.c_str());
	}
}
