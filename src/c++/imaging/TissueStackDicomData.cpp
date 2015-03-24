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

#ifndef	HAVE_CONFIG_H
	#define HAVE_CONFIG_H
#endif
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmimage/dipipng.h"
#include "dcmtk/dcmjpeg/dipijpeg.h"

const bool tissuestack::imaging::TissueStackDicomData::isRaw() const
{
	return false;
}

const bool tissuestack::imaging::TissueStackDicomData::isColor() const
{
	return this->_is_color;
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

	// TODO: writing out file
	DicomImage * img = new DicomImage(potentialDicomFile.c_str());

	unsigned long size = img->getOutputDataSize(8);
	char * buf = new char[size];
	img->getOutputData(buf, size, 8);

	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	Image * image = NULL;
	ImageInfo * imgInfo = NULL;
	image = ConstituteImage(img->getWidth(), img->getHeight(), "I",CharPixel, buf, &exception);
 	if (image == NULL)
	{
		delete [] buf;
		CatchException(&exception);
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not constitute Image!");
	}
 	imgInfo = CloneImageInfo((ImageInfo *)NULL);
 	strcpy(image->filename, (potentialDicomFile + ".magick.png").c_str());
 	WriteImage(imgInfo, image);
 	DestroyImage(image);
 	DestroyImageInfo(imgInfo);
	//img->writePluginFormat(new DiPNGPlugin(), (potentialDicomFile + ".png").c_str());
	//img->writePluginFormat(new DiJPEGPlugin(), (potentialDicomFile + ".jpg").c_str());
	delete img;
	// TODO: remove after writing out file

	DcmFileFormat * dicom = new DcmFileFormat();
	OFCondition status = dicom->loadFile(potentialDicomFile.c_str());
	if (!status.good())
	{
		delete dicom;
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Given dicom file is no good!");
	}

	// we will only allow same series numbers in one zip
	OFString value;
	if (!dicom->getDataset()->findAndGetOFString(DCM_SeriesNumber, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not read series number of dicom file!");
	if (this->_series_number.empty())
		this->_series_number = std::string(value.c_str());
	else if (this->_series_number.compare(value.c_str()) != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We allow only one series per zipped archive of dicom files!");

	this->_dicom_files.push_back(dicom);
}

tissuestack::imaging::TissueStackDicomData::TissueStackDicomData(
	const std::string & filename_of_zip, const std::vector<std::string> & zippedFiles) :
		tissuestack::imaging::TissueStackImageData(filename_of_zip, tissuestack::imaging::FORMAT::DICOM)
{
	for (auto potential_dicom : zippedFiles) // weed out the non .dcm files
	{
		if (potential_dicom.length() < 4)
			continue;

		std::string ext = potential_dicom.substr(potential_dicom.length()-4);
		std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

		if (ext.compare(".DCM") != 0) // ignore all but .dcm
			continue;

		this->addDicomFile(potential_dicom, true);
	}

	// sort according to instance numbers
	std::sort (this->_dicom_files.begin(), this->_dicom_files.end(),
		[](DcmFileFormat * d1, DcmFileFormat * d2) {
		   OFString value;
		   if (!d1->getDataset()->findAndGetOFString(DCM_InstanceNumber, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"Could not read dicom instance number!");
		   unsigned long int d1_ordinal = strtoul(value.c_str(), NULL, 10);
		   if (!d2->getDataset()->findAndGetOFString(DCM_InstanceNumber, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"Could not read dicom instance number!");
		   unsigned long int d2_ordinal = strtoul(value.c_str(), NULL, 10);

		   if (d1_ordinal == d2_ordinal)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"We have 2 same instance numbers for 2 different dicoms!");

		   return (d1 < d2);
	});

	this->initializeDicomImageFromFiles();
}

void tissuestack::imaging::TissueStackDicomData::initializeDicomImageFromFiles()
{
	// now determine whether we have 3D or time series as well as coordinate matrix
	std::vector<unsigned long int> dim_slices;
	std::vector<unsigned long long int> widths;
	std::vector<unsigned long long int> heights;
	std::vector<std::string> orientations;
	std::vector<std::string> steps;
	std::vector<std::string> coords;

	std::string latestOrientation = "";
	unsigned long int counter = 0;
	for (auto dicom : this->_dicom_files)
	{
		OFString value;
		std::string presentOrientation = "";
		if (!dicom->getDataset()->findAndGetOFStringArray(DCM_ImageOrientationPatient, value).good())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"ImageOrientationPatient tag is missing in given dicom!");

		if (latestOrientation.empty() || latestOrientation.compare(value.c_str()) != 0)
		{
			if (!latestOrientation.empty())
				dim_slices.push_back(counter);

			latestOrientation = std::string(value.c_str());
			orientations.push_back(latestOrientation);

			if (!dicom->getDataset()->findAndGetOFStringArray(DCM_ImagePositionPatient, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
							"ImagePositionPatient tag is missing in given dicom!");
			coords.push_back(std::string(value.c_str()));

			if (!dicom->getDataset()->findAndGetOFStringArray(DCM_PixelSpacing, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
							"PixelSpacing tag is missing in given dicom!");
			steps.push_back(std::string(value.c_str()));

			if (!dicom->getDataset()->findAndGetOFString(DCM_Rows, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract number of rows for given dicom!");
			heights.push_back(strtoull(value.c_str(), NULL, 10));

			if (!dicom->getDataset()->findAndGetOFString(DCM_Columns, value).good())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not extract number of columns for given dicom!");
			widths.push_back(strtoull(value.c_str(), NULL, 10));

			this->_plane_index.push_back(counter == 0 ? 0 : counter-1);
		}
		counter++;
	}
	dim_slices.push_back(counter);

	if (this->_plane_index.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Zero Dimensions after dicom file header read!");

	if (this->_plane_index.size() == 1) // time series
	{
		this->initializeDicomTimeSeries(widths, heights, orientations, steps, coords);
		this->dumpImageDataIntoDebugLog();

		return;
	}

	// we go by standard dicom dimension ordering => z, x, y
	// preliminary checks first
	if (this->_plane_index.size() != 3 || widths.size() != 3 || heights.size() != 3 ||
		steps.size() != 3 || coords.size() != 3)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"We need triples for a 3D data dicom!");

	this->initializeDicom3Ddata(dim_slices, widths, heights, orientations, steps, coords);
	this->dumpImageDataIntoDebugLog();
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
	this->addDimension( // time dimension
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("t"),
			0, // bogus offset for now, we'll calculate later
			this->_dicom_files.size(),
			widths[0] * heights[0]));

	this->addCoordinates(coords[0],0);
	this->addCoordinates(coords[0],1);

	this->addSteps(steps[0], orientations[0], 0);
	this->addSteps(steps[0], orientations[0], 1);

	this->generateRawHeader();

	// further dimension info initialization
	this->initializeDimensions(true);
	this->initializeOffsetsForNonRawFiles();
}

inline void tissuestack::imaging::TissueStackDicomData::initializeDicom3Ddata(
	const std::vector<unsigned long int> & dim_slices,
	const std::vector<unsigned long long int> & widths,
	const std::vector<unsigned long long int> & heights,
	const std::vector<std::string> & orientations,
	const std::vector<std::string> & steps,
	const std::vector<std::string> & coords)
{
	// z plane
	unsigned long long int numSlices = heights[1];

	if (dim_slices[0] != numSlices)
		void(); // TODO: implement possible correction

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

	if (dim_slices[1] != numSlices)
		void(); // TODO: implement possible correction

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

	if (dim_slices[2] != numSlices)
		void(); // TODO: implement possible correction

	this->addDimension(
		new tissuestack::imaging::TissueStackDataDimension(
			std::string("y"),
			0, // bogus offset for now, we'll calculate later
			numSlices,
			widths[2] * heights[2]));
	this->addCoordinates(coords[0],1);
	this->addSteps(steps[0], orientations[0], 1);

	this->generateRawHeader();
	// further dimension info initialization
	this->initializeDimensions(true);
	this->initializeOffsetsForNonRawFiles();
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
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::DICOM)
{
	this->addDicomFile(filename);
	// TODO: add dimensions x, y (read dicom dimensions)
}

tissuestack::imaging::TissueStackDicomData::~TissueStackDicomData()
{
	for (auto dicom : this->_dicom_files)
		if (dicom != nullptr)
			delete dicom;
}
