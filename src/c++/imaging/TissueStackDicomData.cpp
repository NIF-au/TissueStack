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

const tissuestack::imaging::DICOM_PLANAR_ORIENTATION tissuestack::imaging::TissueStackDicomData::getPlanarOrientation() const
{
	return this->_planar_orientation;
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
		const std::string tmpDir =
			tissuestack::imaging::TissueStackImageData::assembleTemporaryDirectoryForZipFiles(this->getFileName());

		potentialDicomFile = tmpDir + "/" + potentialDicomFile;
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
				ext.compare(".IMA") != 0 &&
				ext.find(".") != std::string::npos) // ignore all but .dcm, .ima or blank extensions
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
	unsigned long int planeCounter = 0;
	for (auto dicom : this->_dicom_files)
	{
		if (latestOrientation.empty() || latestOrientation.compare(dicom->getImageOrientation()) != 0)
		{
			if (!latestOrientation.empty())
			{
				this->_plane_number_of_files.push_back(planeCounter);
				planeCounter = 0; // reset
			}

			latestOrientation = dicom->getImageOrientation();
			latestWidth = dicom->getWidth();
			latestHeight = dicom->getHeight();
			orientations.push_back(latestOrientation);

			coords.push_back(dicom->getImagePositionPatient());
			steps.push_back(dicom->getPixelSpacing());

			heights.push_back(dicom->getHeight());
			widths.push_back(dicom->getWidth());

			this->_plane_index.push_back(counter == 0 ? 0 : counter);
		} else if (latestWidth != dicom->getWidth() || latestHeight != dicom->getHeight())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Slices within same dimension have to have same width and height!");

		planeCounter++;
		counter++;
	}
	this->_plane_number_of_files.push_back(planeCounter);

	if (this->_plane_index.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Zero Dimensions after dicom file header read!");

	if (this->_dicom_files.size() == 1)
	{
		this->initializeSingleDicomFile(this->_dicom_files[0]);
		return;
	}

	// here are the cases that we deal with as a 'time series':
	// -) mosaic
	// -) if the data type is declared 2D and the ImagesInSeries tag
	//    is present, being a number that is a multiple of the overall number of files
	// -) we have more than 3 different orientations
	if (this->_plane_index.size() > 3 ||
		(this->_plane_index.size() == 1 &&
			(this->_dicom_files[0]->isMosaic() ||
				(this->_dicom_files[0]->getNumberOfImagesInSeriesOrAcquision() != 0 &&
				this->_dicom_files[0]->getNumberOfImagesInSeriesOrAcquision() != this->_dicom_files.size() &&
				this->_dicom_files.size() % this->_dicom_files[0]->getNumberOfImagesInSeriesOrAcquision() == 0))))
	{
		this->initializeDicomTimeSeries(widths, heights, steps, coords);
		return;
	}

	// these are the cases where we have -on the surface- 2D but we'll try to make it 3D
	// by reconstruction taking the data from the slices into 1 direction
	if (this->_plane_index.size() == 1)
	{
		this->initializePartialDicom3Ddata(widths, heights, steps, coords);
		return;
	}

	// we have 3 planes
	if (this->_plane_index.size() == 3)
	{
		this->initializeDicom3Ddata(widths, heights, steps, coords);
		return;
	}

	THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
		"Dont't know what to do with this dicom ....!")
}

inline void tissuestack::imaging::TissueStackDicomData::initializeDicomTimeSeries(
	const std::vector<unsigned long long int> & widths,
	const std::vector<unsigned long long int> & heights,
	const std::vector<std::string> & steps,
	const std::vector<std::string> & coords)
{
	this->_type = tissuestack::imaging::DICOM_TYPE::TIME_SERIES;

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

	this->addSteps(steps[0], 0);
	this->addSteps(steps[0], 1);

	// further dimension info initialization (order of function calls matter!)
	this->detectAndCorrectFor2DData();
	this->generateRawHeader();
	this->initializeOffsetsForNonRawFiles();
}

const tissuestack::imaging::DICOM_PLANAR_ORIENTATION tissuestack::imaging::TissueStackDicomData::findPlanarOrientationOfDicomFile(const tissuestack::imaging::DicomFileWrapper * dicom) const
{
	if (dicom == nullptr)
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED;

	const std::string orientation =
		dicom->getImageOrientation();
	if (orientation.empty())
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED;

	const std::vector<std::string> cosines =
		tissuestack::utils::Misc::tokenizeString(orientation, '\\');

	if (cosines.size() != 6)
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED;

	const char xAxis =
		this->determinePlanarOrientation0(
			atof(cosines[0].c_str()), atof(cosines[1].c_str()),atof(cosines[2].c_str()));
	const char yAxis = this->determinePlanarOrientation0(
			atof(cosines[3].c_str()), atof(cosines[4].c_str()),atof(cosines[5].c_str()));

	if (xAxis == '\0' || yAxis == '\0')
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED;

	if	(((xAxis == 'R' || xAxis == 'L') && (yAxis == 'A' || yAxis == 'P')) ||
			((xAxis == 'A' || xAxis == 'P') && (yAxis == 'R' || yAxis == 'L'))			)
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::AXIAL;
	else if	(((xAxis == 'R' || xAxis == 'L') && (yAxis == 'H' || yAxis == 'F')) ||
			((xAxis == 'H' || xAxis == 'F') && (yAxis == 'R' || yAxis == 'L'))			)
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::CORONAL;
	else if	(((xAxis == 'A' || xAxis == 'P') && (yAxis == 'H' || yAxis == 'F')) ||
			((xAxis == 'H' || xAxis == 'F') && (yAxis == 'A' || yAxis == 'P'))			)
		return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::SAGITTAL;

	return tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED;
}

void tissuestack::imaging::TissueStackDicomData::determinePlanarOrientation()
{
	if (this->_plane_index.size() != 1) // don't care for orientation if we don't have to reconstruct 3D
		return;

	this->_planar_orientation =
		this->findPlanarOrientationOfDicomFile(this->getDicomFileWrapper(0));
}

const char tissuestack::imaging::TissueStackDicomData::determinePlanarOrientation0(float x, float y, float z) const
{
	char orientationX = x < 0 ? 'R' : 'L';
	char orientationY = y < 0 ? 'A' : 'P';
	char orientationZ = z < 0 ? 'F' : 'H';

	float absX = fabsf(x);
	float absY = fabsf(y);
	float absZ = fabsf(z);

	if (absX > 0.8 && absX > absY && absX > absZ)
		return orientationX;

	if (absY > 0.8 && absY > absX && absY > absZ)
		return orientationY;

	if (absZ > 0.8 && absZ > absX && absZ > absY)
		return orientationZ;

	return '\0';
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
	this->addSteps(dicom->getPixelSpacing(), 0);
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // bogus offset for now, we'll calculate later
			dicom->getHeight(),
			0));
	this->addCoordinates(dicom->getImagePositionPatient(), 1);
	this->addSteps(dicom->getPixelSpacing(), 1);

	this->generateRawHeader();

	// further dimension info initialization
	this->initializeDimensions(true);
	this->initializeOffsetsForNonRawFiles();
}

inline void tissuestack::imaging::TissueStackDicomData::initializePartialDicom3Ddata(
		const std::vector<unsigned long long int> & widths,
		const std::vector<unsigned long long int> & heights,
		const std::vector<std::string> & steps,
		const std::vector<std::string> & coords)
	{
	this->_type = tissuestack::imaging::DICOM_TYPE::VOLUME_TO_BE_RECONSTRUCTED;
	this->determinePlanarOrientation();

	std::string planeIdentifiers[3] = {"x","y", "z"};
	unsigned long long int planeSliceSizes[3] = { // SAGGITAL default
			this->_dicom_files.size(),
			widths[0],
			heights[0]
	};

	if (this->_planar_orientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::AXIAL)
	{
		planeSliceSizes[0] = widths[0];
		planeSliceSizes[1] = heights[0];
		planeSliceSizes[2] = this->_dicom_files.size();
	} else if (this->_planar_orientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::CORONAL)
	{
		planeSliceSizes[0] = widths[0];
		planeSliceSizes[1] = this->_dicom_files.size();
		planeSliceSizes[2] = heights[0];
	}

	for (unsigned short i=0;i<3;i++)
	{
		this->addDimension(
			new tissuestack::imaging::TissueStackDataDimension(
				planeIdentifiers[i],
				0, // bogus offset for now, we'll calculate later
				planeSliceSizes[i],
				0)); // we'll calculate later
		this->addCoordinates(coords[0],i);
		this->addSteps(steps[0], i > 1 ? 0 : i);

	}

	this->initializeDimensions(true);
	this->generateRawHeader();
	this->initializeOffsetsForNonRawFiles();
}

inline void tissuestack::imaging::TissueStackDicomData::initializeDicom3Ddata(
	const std::vector<unsigned long long int> & widths,
	const std::vector<unsigned long long int> & heights,
	const std::vector<std::string> & steps,
	const std::vector<std::string> & coords)
{
	// preliminary checks whether we can sensibly use series with 3 planes
	short saggitalDimensionIndex = -1;
	short coronalDimensionIndex = -1;
	short axialDimensionIndex = -1;

	for (unsigned short i=0;i<3;i++)
	{
		const tissuestack::imaging::DICOM_PLANAR_ORIENTATION planeOrientation =
			this->findPlanarOrientationOfDicomFile(this->getDicomFileWrapper(this->getPlaneIndex(i)));

		if (planeOrientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::UNDETERMINED)
			break;
		if (planeOrientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::AXIAL)
			axialDimensionIndex = i;
		else if (planeOrientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::CORONAL)
			coronalDimensionIndex = i;
		else if (planeOrientation == tissuestack::imaging::DICOM_PLANAR_ORIENTATION::SAGITTAL)
			saggitalDimensionIndex = i;
	}

	// without all of the planar orientations we'd rather not make a 3D volume given 3 planes
	if (saggitalDimensionIndex < 0 || coronalDimensionIndex < 0 || axialDimensionIndex < 0)
	{
		// we have to check whether we have a consistent width and height
		if (!this->checkWidthAndHeightConsistencyOfDicoms(0, this->_dicom_files.size()-1))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Dicom Time Series contains a file whose width & height does not match the others!");

		this->initializeDicomTimeSeries(widths, heights, steps, coords);
		return;
	}

	// now the width/height have to match the slice numbers
	const tissuestack::imaging::DicomFileWrapper * axialDimension =
		this->getDicomFileWrapper(this->getPlaneIndex(axialDimensionIndex));
	const tissuestack::imaging::DicomFileWrapper * coronalDimension =
		this->getDicomFileWrapper(this->getPlaneIndex(coronalDimensionIndex));
	const tissuestack::imaging::DicomFileWrapper * saggitalDimension =
			this->getDicomFileWrapper(this->getPlaneIndex(saggitalDimensionIndex));

	if (axialDimension->getWidth() != this->getNumberOfFiles(saggitalDimensionIndex) ||
		axialDimension->getHeight() != this->getNumberOfFiles(coronalDimensionIndex) ||
		coronalDimension->getWidth() != this->getNumberOfFiles(saggitalDimensionIndex) ||
		coronalDimension->getHeight() != this->getNumberOfFiles(axialDimensionIndex) ||
		saggitalDimension->getWidth() != this->getNumberOfFiles(coronalDimensionIndex) ||
		saggitalDimension->getHeight() != this->getNumberOfFiles(axialDimensionIndex))
	{
		// we have to check whether we have a consistent width and height
		if (!this->checkWidthAndHeightConsistencyOfDicoms(0, this->_dicom_files.size()-1))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Dicom Time Series contains a file whose width & height does not match the others!");

		this->initializeDicomTimeSeries(widths, heights, steps, coords);
		return;
	}

	// x plane (saggital)
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("x"),
			0, // we'll calculate later
			this->getNumberOfFiles(saggitalDimensionIndex),
			0));// we'll calculate later
	this->addCoordinates(coords[saggitalDimensionIndex],0);
	this->addSteps(steps[saggitalDimensionIndex], 0);

	// y plane (coronal)
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // we'll calculate later
			this->getNumberOfFiles(coronalDimensionIndex),
			0));// we'll calculate later
	this->addCoordinates(coords[coronalDimensionIndex],1);
	this->addSteps(steps[coronalDimensionIndex], 0);

	// z plane (axial)
	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("z"),
			0, // we'll calculate later
			this->getNumberOfFiles(axialDimensionIndex),
			0));// we'll calculate later
	this->addCoordinates(coords[axialDimensionIndex],2);
	this->addSteps(steps[axialDimensionIndex], 0);

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

	this->addStep(atof(s[index].c_str()));
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

	std::string fileNameAllUpperCase = this->getFileName();
	std::transform(fileNameAllUpperCase.begin(), fileNameAllUpperCase.end(), fileNameAllUpperCase.begin(), toupper);

	if (fileNameAllUpperCase.rfind(".ZIP") != std::string::npos)
	{
		const std::string tmpDir =
			tissuestack::imaging::TissueStackImageData::assembleTemporaryDirectoryForZipFiles(this->getFileName());
		rmdir(tmpDir.c_str());
	}
}

const unsigned long int tissuestack::imaging::TissueStackDicomData::getNumberOfFiles(const unsigned short dimension_index) const
{
	if (this->_plane_number_of_files.empty() || dimension_index >= this->_plane_number_of_files.size())
		return 0;

	return this->_plane_number_of_files[dimension_index];
}

const unsigned long int tissuestack::imaging::TissueStackDicomData::getTotalNumberOfDicomFiles() const
{
	return this->_dicom_files.size();
}

const unsigned long int tissuestack::imaging::TissueStackDicomData::getPlaneIndex(const unsigned short dimension_index) const
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

const bool tissuestack::imaging::TissueStackDicomData::checkWidthAndHeightConsistencyOfDicoms(
		const unsigned short start, const unsigned short end) const
{
	if (this->_dicom_files.empty() || this->_dicom_files.size() == 1 || end-start == 1)
		return true;

	if (start >= end || start >= this->_dicom_files.size() || end >= this->_dicom_files.size())
		return false;

	unsigned long long int width = -1;
	unsigned long long int height = -1;
	for (int j=start;j<=end;j++)
	{
		const tissuestack::imaging::DicomFileWrapper * dicom =
			this->getDicomFileWrapper(j);
		if (dicom == nullptr)
			return false;

		if (j==start)
		{
			width = dicom->getWidth();
			height = dicom->getHeight();
			continue;
		}

		if (dicom->getWidth() != width ||
				dicom->getHeight() != height)
			return false;
	}

	return true;
}

void tissuestack::imaging::TissueStackDicomData::registerDcmtkDecoders() const
{
	DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation);
	DJLSDecoderRegistration::registerCodecs();
	DcmRLEDecoderRegistration::registerCodecs();
}

void tissuestack::imaging::TissueStackDicomData::deregisterDcmtkDecoders() const
{
	DJDecoderRegistration::cleanup();
	DJLSDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
}
