#include "imaging.h"

tissuestack::imaging::TissueStackRawData::~TissueStackRawData()
{

}

const bool tissuestack::imaging::TissueStackRawData::isRaw() const
{
	return true;
}

tissuestack::imaging::TissueStackRawData::TissueStackRawData(const std::string & filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::MINC)
{
	char header[15];
	memset(header, '\0', 15);
	int bRead = pread(this->getFileDescriptor(), header, 15, 0);

	// first a general header test
	// read failed, try again
	if (bRead <= 0)
		bRead = pread(this->getFileDescriptor(), header, 15, 0);
	if (bRead <= 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Supposed RAW file could not be read!");

	if (strncmp(header, "@IaMraW@", 8) != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Supposed RAW file does not start with expected header!");

	int startOfLengthInformation = 9;
	int i = startOfLengthInformation, pipePos = -1;

	if (header[8] == 'V') // we have a non legacy raw file, extract exact version
	{
		while (i < 15)
		{
			if (header[i] == '|')
			{
				pipePos = i;
				break;
			}
			i++;
		}
		unsigned short versionLength = pipePos-startOfLengthInformation;
		if (versionLength <=0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Non Legacy RAW file need to have a version number!");

		char tmp[versionLength];
		memset(tmp, '\0', versionLength);
		this->setRawVersion(atoi(tmp));
		startOfLengthInformation = pipePos;
	}

	// read in rest of header after finding its length
	memset(header, '\0', 15);
	bRead = pread(this->getFileDescriptor(), header, 15, startOfLengthInformation);
	if (bRead <= 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not read header length of RAW file!");
	i = 0; pipePos = -1;
	while (i < 15)
	{
		if (header[i] == '|')
		{
			pipePos = i;
			break;
		}
		i++;
	}
	if (pipePos <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not read header length of RAW file!");

	char tmp[pipePos+2];
	memcpy(tmp, header, pipePos+1);
	tmp[pipePos+1] = '\0';
	int headerLength = atoi(tmp);

	char extendedHeader[headerLength];
	memset(extendedHeader, '\0', headerLength);
	bRead = pread(this->getFileDescriptor(), extendedHeader, headerLength-1, startOfLengthInformation + pipePos + 1);
	if (bRead <= 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Could not read header content of RAW file!");

	this->_totalHeaderLength = static_cast<unsigned int>(startOfLengthInformation + pipePos + 1 + headerLength);
	const std::string fullHeader(extendedHeader, headerLength-1);

	// delegate parsing
	//tissuestack::logging::TissueStackLogger::instance()->info("Parsing header information for RAW file: %s...\n", filename.c_str());
	this->parseHeader(fullHeader);

	// calculate the supposed file size
	this->_supposedFileSizeInButes = this->_totalHeaderLength;

	for (auto dim : this->getDimensionMap())
	{
		const TissueStackDataDimension * d = dim.second; // LEGACY UNSIGNED CHAR HAS 1 BYTE REPRESENTATION
		if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY
					&& this->_raw_type == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT)
			this->_supposedFileSizeInButes +=
					static_cast<unsigned long long int>((d->getNumberOfSlices()*d->getSliceSize()));
		else // LEGACY 3BYTE RGB and V1 RESPRESENTATION
			this->_supposedFileSizeInButes +=
					static_cast<unsigned long long int>(
							static_cast<unsigned long long int>(3) *
							static_cast<unsigned long long int>((d->getNumberOfSlices()*d->getSliceSize())));

	}
}

void tissuestack::imaging::TissueStackRawData::parseHeader(const std::string & header)
{
	const std::vector<std::string> headerTokens = tissuestack::utils::Misc::tokenizeString(header, '|');

	// preliminary header token check
	if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY && headerTokens.size() < 13)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "A Legacy Tissue Stack RAW file will need at least 13 header bits!");
	if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::V1 && headerTokens.size() < 5)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "A V1 Tissue Stack RAW file will need at least 5 header bits!");

	// for V1 and up: we don't have a separate dimension number token at the beginning
	unsigned short count =
			(this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY) ? 0 : 1;

	int numOfDims = 0;
	std::vector<int> dims;
	std::vector<std::string> tmpTokenString;

	for (const std::string t : headerTokens)
	{
		switch (count)
		{
			case 0:
				// LEGACY RAW: the number of dimensions
				numOfDims = atoi(t.c_str());
				break;
			case 1:
				// the dimensions
				tmpTokenString = tissuestack::utils::Misc::tokenizeString(t, ':');
				for (std::string s : tmpTokenString)
					dims.push_back(atoi(s.c_str()));
				if (this->_raw_version != tissuestack::imaging::RAW_FILE_VERSION::LEGACY)
					numOfDims = dims.size();
				if (numOfDims < 2)
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "A Tissue Stack RAW file has to be 2D at a minimum!");
				break;
			case 2:
				tmpTokenString = tissuestack::utils::Misc::tokenizeString(t, ':');
				for (std::string s : tmpTokenString)
					this->addCoordinate(strtof(s.c_str(), NULL));
				break;
			case 3: // the steps
				tmpTokenString = tissuestack::utils::Misc::tokenizeString(t, ':');
				for (std::string s : tmpTokenString)
					this->addStep(strtof(s.c_str(), NULL));
				tmpTokenString.clear();
				break;
			case 4:
			case 5:
			case 6:
				// for a LEGACY RAW the dimension names are scattered over then next tokens
				if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY)
				{
					if (numOfDims == 2 && count == 6)
						break;
					tmpTokenString.push_back(t);
				} else if (count == 4) // for a V1 and up: we have them all in there
				{
					tmpTokenString = tissuestack::utils::Misc::tokenizeString(t, ':');
					count=6; // fast forward to 6 to stay compatible with switch logic
				}
				break;
			case 7:
				// LEGACY RAW: redundant short dim names which we skip
				if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY)
					break;

				// for V1 RAW: here is the original format
				if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::V1)
					this->setFormat(atoi(t.c_str()));
				break;
			case 8: // LEGACY RAW: dimension short names (redundant and unused)
			case 9:
				if (this->_raw_version == tissuestack::imaging::RAW_FILE_VERSION::LEGACY && numOfDims == 2 && count == 8)
					count++;
				break;
			case 10: // LEGACY RAW: slice sizes that have already been calculated
				break;
			case 11: // LEGACY RAW: max slice size which is easy enough to get to not waste header space
				break;
			case 12: // LEGACY RAW: offsets that have already been calculated
				break;
			case 13:
				// LEGACY RAW (optional): here we find the original format
				this->setFormat(atoi(t.c_str()));
				break;
			case 14:
				// LEGACY RAW (optional): here we find the raw type 8 vs 26 bit representation
				this->setRawType(atoi(t.c_str()));
				break;
			default:
				break;
		}
		count++;
	}

	// finally we have everything we need to construct and add our dimension object
	unsigned short j = 0;
	unsigned long long int offset[4] =
	{
			this->_totalHeaderLength,
			this->_totalHeaderLength,
			this->_totalHeaderLength,
			this->_totalHeaderLength
	};
	for (std::string s : tmpTokenString)
	{
		unsigned long long int sliceSize = 0;
		int k=0;
		for (int size : dims)
		{
			// use first that is not the present index j
			if (k == j)
			{
				k++;
				continue;
			}
			if (sliceSize == 0) // first dimension size to participate in the product
				sliceSize = static_cast<unsigned long long int>(size);
			else // further dimension sizes to be multiplied into the product
				sliceSize *= static_cast<unsigned long long int>(size);

			k++;
		}
		// keep track of dimension offset
		// and take into account the raw type (8 vs 24 bit representation)
		if (j>0)
			offset[j] =
				offset[j-1] +
				sliceSize*static_cast<long long unsigned int>(dims[j])*
				static_cast<unsigned long long int>(
						this->_raw_type == tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT ? 1 : 3);

		this->addDimension(
				new tissuestack::imaging::TissueStackDataDimension(
					s,
					offset[j],
					dims[j],
					sliceSize
		));
		j++;
	}
}

void tissuestack::imaging::TissueStackRawData::setRawVersion(int version)
{
	switch (version)
	{
		case tissuestack::imaging::RAW_FILE_VERSION::LEGACY:
			this->_raw_version = tissuestack::imaging::RAW_FILE_VERSION::LEGACY;
			break;
		case tissuestack::imaging::RAW_FILE_VERSION::V1:
			this->_raw_version = tissuestack::imaging::RAW_FILE_VERSION::V1;
			break;
		default:
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Incompatible Raw File Version Enum!");
			break;
	}
}


void tissuestack::imaging::TissueStackRawData::setRawType(int type)
{
	switch (type)
	{
		case tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT:
			this->_raw_type = tissuestack::imaging::RAW_TYPE::UCHAR_8_BIT;
			break;
		case tissuestack::imaging::RAW_TYPE::RGB_24BIT:
			this->_raw_type = tissuestack::imaging::RAW_TYPE::RGB_24BIT;
			break;
		default:
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Incompatible Raw Type Enum!");
			break;
	}
}

const unsigned long long int tissuestack::imaging::TissueStackRawData::getFileSizeInBytes() const
{
	struct stat buf;

	if (stat(this->getFileName().c_str(), &buf) == -1)
		return 0;

	return buf.st_size;
}

const unsigned long long int tissuestack::imaging::TissueStackRawData::getSupposedFileSizeInBytes() const
{
	return this->_supposedFileSizeInButes;
}
