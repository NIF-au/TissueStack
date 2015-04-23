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

const bool tissuestack::imaging::TissueStackDicomData::isRaw() const
{
	return false;
}

const tissuestack::imaging::DICOM_TYPE tissuestack::imaging::TissueStackDicomData::getType() const
{
	return this->_type;
}

const tissuestack::imaging::DicomFileWrapper * tissuestack::imaging::TissueStackDicomData::getDicomFileWrapper(unsigned int index) const
{
	if (index >= this->_dicom_files.size())
		return nullptr;

	return this->_dicom_files[index];
}

void tissuestack::imaging::TissueStackDicomData::addDicomFile(const std::string & file, const bool withinZippedArchive)
{
	std::string potentialDicomFile = file;

	if (withinZippedArchive)
	{
		potentialDicomFile = std::string("/tmp/") + potentialDicomFile;
		if (!tissuestack::utils::Misc::extractZippedFileFromArchive(
			this->getFileName(), file, potentialDicomFile, true))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to extract dicom file from zip to location: '/tmp'!");
	}

	std::unique_ptr<tissuestack::imaging::DicomFileWrapper> dicom_ptr(
		tissuestack::imaging::DicomFileWrapper::createWrappedDicomFile(potentialDicomFile, withinZippedArchive ? true : false));

	// we will only allow same series numbers in one zip
	if (this->_series_number.empty())
		this->_series_number = dicom_ptr->getSeriesNumber();
	else if (this->_series_number.compare(dicom_ptr->getSeriesNumber()) != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We allow only one series per zipped archive of dicom files!");

 	this->_dicom_files.push_back(dicom_ptr.release());
}

tissuestack::imaging::TissueStackDicomData::TissueStackDicomData(
	const std::string & filename_of_zip, const std::vector<std::string> & zippedFiles) :
		tissuestack::imaging::TissueStackImageData(filename_of_zip, tissuestack::imaging::FORMAT::DICOM),
		_type(tissuestack::imaging::DICOM_TYPE::VOLUME)
{
	for (auto potential_dicom : zippedFiles) // weed out the non .dcm files
	{
		if (potential_dicom.length() < 4)
			continue;

		std::string ext = potential_dicom.substr(potential_dicom.length()-4);
		std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

		if (ext.compare(".DCM") != 0 &&
				ext.compare(".IMG") != 0 &&
				ext.find(".") != std::string::npos) // ignore all but .dcm, .img or blank extensions
			continue;

		this->addDicomFile(potential_dicom, true);
	}

	// sort according to instance numbers
	std::sort (this->_dicom_files.begin(), this->_dicom_files.end(),
		[](DicomFileWrapper * d1, DicomFileWrapper * d2) {
		   if (d1->getInstanceNumber() == d2->getInstanceNumber())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"We have 2 same instance numbers for 2 different dicoms!");

		   return (d1->getInstanceNumber() < d2->getInstanceNumber());
	});

	this->initializeDicomImageFromFiles();
}

void tissuestack::imaging::TissueStackDicomData::initializeDicomImageFromFiles()
{
	// now determine whether we have 3D or time series as well as coordinate matrix
	std::vector<unsigned long long int> widths;
	std::vector<unsigned long long int> heights;
	std::vector<std::string> orientations;
	std::vector<std::string> steps;
	std::vector<std::string> coords;

	std::string latestOrientation = "";
	unsigned long long int latestWidth = 0;
	unsigned long long int latestHeight = 0;
	unsigned long int counter = 0;
	for (auto dicom : this->_dicom_files)
	{
		if (latestOrientation.empty() || latestOrientation.compare(dicom->getImageOrientation()) != 0)
		{
			if (!latestOrientation.empty())
				this->_plane_number_of_files.push_back(counter);

			latestOrientation = dicom->getImageOrientation();
			latestWidth = dicom->getWidth();
			latestHeight = dicom->getHeight();
			orientations.push_back(latestOrientation);

			coords.push_back(dicom->getImagePositionPatient());
			steps.push_back(dicom->getPixelSpacing());

			heights.push_back(dicom->getHeight());
			widths.push_back(dicom->getWidth());

			this->_plane_index.push_back(counter == 0 ? 0 : counter-1);
		} else if (latestWidth != dicom->getWidth() || latestHeight != dicom->getHeight())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Slices within same dimension have to have same width and height!");

		counter++;
	}
	this->_plane_number_of_files.push_back(counter);

	if (this->_plane_index.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Zero Dimensions after dicom file header read!");

	if (this->_dicom_files.size() == 1)
	{
		this->initializeSingleDicomFile(this->_dicom_files[0]);

		return;
	}

	if (this->_plane_index.size() == 1) // time series
	{
		this->_type = tissuestack::imaging::DICOM_TYPE::TIME_SERIES;
		this->initializeDicomTimeSeries(widths, heights, orientations, steps, coords);

		return;
	}

	// we go by standard dicom dimension ordering => z, x, y
	// preliminary checks first
	if (this->_plane_index.size() != 3 || widths.size() != 3 || heights.size() != 3 ||
		steps.size() != 3 || coords.size() != 3)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We need triples for a 3D data dicom!");

	this->initializeDicom3Ddata(widths, heights, orientations, steps, coords);
}

inline void tissuestack::imaging::TissueStackDicomData::initializeDicomTimeSeries(
	const std::vector<unsigned long long int> & widths,
	const std::vector<unsigned long long int> & heights,
	const std::vector<std::string> & orientations,
	const std::vector<std::string> & steps,
	const std::vector<std::string> & coords)
{
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("x"),
			0, // bogus offset for now, we'll calculate later
			widths[0],
			widths[0] * heights[0]));
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // bogus offset for now, we'll calculate later
			heights[0],
			widths[0] * heights[0]));
	tissuestack::imaging::TissueStackDataDimension * timeDim =
		new tissuestack::imaging::TissueStackDataDimension(
				std::string("time"),
				0, // bogus offset for now, we'll calculate later
				this->_dicom_files.size(),
				widths[0] * heights[0]);
	timeDim->setWidthAndHeight(widths[0], heights[0], widths[0], heights[0]);
	this->addDimension(timeDim);

	this->addCoordinates(coords[0],0);
	this->addCoordinates(coords[0],1);

	this->addSteps(steps[0], orientations[0], 0);
	this->addSteps(steps[0], orientations[0], 1);

	// further dimension info initialization (order of function calls matter!)
	this->detectAndCorrectFor2DData();
	this->generateRawHeader();
	this->initializeOffsetsForNonRawFiles();
}

inline void tissuestack::imaging::TissueStackDicomData::initializeSingleDicomFile(const tissuestack::imaging::DicomFileWrapper * dicom)
{
	if (dicom == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException,
			"Dicom wrapper object is NULL!");

	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("x"),
			0, // bogus offset for now, we'll calculate later
			dicom->getWidth(),
			0));
	this->addCoordinates(dicom->getImagePositionPatient(), 0);
	this->addSteps(dicom->getPixelSpacing(), dicom->getImageOrientation(), 0);
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // bogus offset for now, we'll calculate later
			dicom->getHeight(),
			0));
	this->addCoordinates(dicom->getImagePositionPatient(), 1);
	this->addSteps(dicom->getPixelSpacing(), dicom->getImageOrientation(), 1);

	this->generateRawHeader();

	// further dimension info initialization
	this->initializeDimensions(true);
	this->initializeOffsetsForNonRawFiles();
}



inline void tissuestack::imaging::TissueStackDicomData::initializeDicom3Ddata(
	const std::vector<unsigned long long int> & widths,
	const std::vector<unsigned long long int> & heights,
	const std::vector<std::string> & orientations,
	const std::vector<std::string> & steps,
	const std::vector<std::string> & coords)
{
	// z plane
	unsigned long long int numSlices = heights[1];
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("z"),
			0, // bogus offset for now, we'll calculate later
			numSlices,
			widths[0] * heights[0]));
	this->addCoordinates(coords[1],2);
	this->addSteps(steps[1], orientations[1], 1);

	// x plane
	numSlices = widths[0];
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("x"),
			0, // bogus offset for now, we'll calculate later
			numSlices,
			widths[1] * heights[1]));
	this->addCoordinates(coords[0],0);
	this->addSteps(steps[0], orientations[0], 0);

	// y plane
	numSlices = heights[0];
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // bogus offset for now, we'll calculate later
			numSlices,
			widths[2] * heights[2]));
	this->addCoordinates(coords[0],1);
	this->addSteps(steps[0], orientations[0], 1);

	this->initializeDimensions(true, false);
	this->initializeOffsetsForNonRawFiles();
	this->generateRawHeader();
}

inline void tissuestack::imaging::TissueStackDicomData::addCoordinates(
		const std::string & coords, const unsigned short index)
{
	if (coords.empty() || index > 2)
		return;

	const std::vector<std::string> c =
		tissuestack::utils::Misc::tokenizeString(coords, '\\');

	if (c.size() != 3 || index >= c.size())
	{
		this->addCoordinate(0);
		return;
	}

	this->addCoordinate(atof(c[index].c_str()));
}

inline void tissuestack::imaging::TissueStackDicomData::addSteps(
		const std::string & steps,
		const std::string & orientations,
		const unsigned short index)
{
	if (steps.empty() || index > 2)
		return;
	const std::vector<std::string> s =
		tissuestack::utils::Misc::tokenizeString(steps, '\\');

	if (s.size() != 2 || index >= s.size())
	{
		this->addStep(1);
		return;
	}

	float step = atof(s[index].c_str());

	const std::vector<std::string> o =
		tissuestack::utils::Misc::tokenizeString(orientations, '\\');
	if (o.size() == 6 && index < 2)
	{
		for(unsigned short i=index*3;i<index*3+3;i++)
		{
			const float oTmp =  atof(o[i].c_str());
			if (oTmp != 0 && oTmp < 0)
			{
				step = -step;
				break;
			}
		}
	}

	this->addStep(step);
}

tissuestack::imaging::TissueStackDicomData::TissueStackDicomData(const std::string & filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::DICOM),
		_type(tissuestack::imaging::DICOM_TYPE::SINGLE_IMAGE)
{
	this->addDicomFile(filename);
	this->initializeSingleDicomFile(this->_dicom_files[0]);
}

tissuestack::imaging::TissueStackDicomData::~TissueStackDicomData()
{
	for (auto dicom : this->_dicom_files)
		if (dicom != nullptr)
			delete dicom;
}

const unsigned long int tissuestack::imaging::TissueStackDicomData::getNumberOfFiles(const unsigned short dimension_index)
{
	if (this->_plane_number_of_files.empty() || dimension_index >= this->_plane_number_of_files.size())
		return 0;

	return this->_plane_number_of_files[dimension_index];
}

const unsigned long int tissuestack::imaging::TissueStackDicomData::getPlaneIndex(const unsigned short dimension_index)
{
	if (this->_plane_index.empty() || dimension_index >= this->_plane_index.size())
		return 0;

	return this->_plane_index[dimension_index];
}

void tissuestack::imaging::TissueStackDicomData::writeDicomDataAsPng(tissuestack::imaging::DicomFileWrapper * dicom)
{
	if (dicom == nullptr)
		return;


	this->registerDcmtkDecoders();
	const unsigned char * data = dicom->getData();
	this->deregisterDcmtkDecoders();

	if (data == nullptr)
		return;

	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	Image * image = NULL;

	image = ConstituteImage(
		dicom->getWidth(),
		dicom->getHeight(),
		"RGB",
		CharPixel,
		data, &exception);

 	if (image == NULL)
	{
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}
	delete [] data;

	ImageInfo * imgInfo = CloneImageInfo((ImageInfo *)NULL);
 	strcpy(image->filename, (dicom->getFileName() + ".png").c_str());

 	WriteImage(imgInfo, image);

 	DestroyImage(image);
 	DestroyImageInfo(imgInfo);

 	//DicomImage * img = new DicomImage(potentialDicomFile.c_str());
 	//img->writePluginFormat(new DiPNGPlugin(), (potentialDicomFile + ".png").c_str());
 	//delete img;
	//img->writePluginFormat(new DiJPEGPlugin(), (potentialDicomFile + ".jpg").c_str());
}

void tissuestack::imaging::TissueStackDicomData::registerDcmtkDecoders()
{
	DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation);
	DJLSDecoderRegistration::registerCodecs();
	DcmRLEDecoderRegistration::registerCodecs();
}

void tissuestack::imaging::TissueStackDicomData::deregisterDcmtkDecoders()
{
	DJDecoderRegistration::cleanup();
	DJLSDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
}
