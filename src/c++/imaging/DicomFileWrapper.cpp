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
	#include "dcmtk/dcmdata/dctk.h"
	#include "dcmtk/dcmimgle/dcmimage.h"
	#include "dcmtk/dcmimage/dipipng.h"
	#include "dcmtk/dcmjpeg/dipijpeg.h"
#endif

tissuestack::imaging::DicomFileWrapper * tissuestack::imaging::DicomFileWrapper::createWrappedDicomFile(
	const std::string filename, const bool isTempFile)
{
	return new tissuestack::imaging::DicomFileWrapper(filename, isTempFile);
}

tissuestack::imaging::DicomFileWrapper::DicomFileWrapper(const std::string filename, const bool isTempFile) :
	_file_name(filename), _isTemp(isTempFile)
{
	if (!tissuestack::utils::System::fileExists(filename))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Supposed Dicom File does not exist in that location!");

	// load file to read header tags
	DcmFileFormat dicomFormat;
	OFCondition status = dicomFormat.loadFile(filename.c_str());
	if (!status.good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Given dicom file is no good!");

	// read mandatory header tags needed later
	OFString value;
	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_SeriesNumber, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not read series number of dicom file!");
	this->_series_number = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_InstanceNumber, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not read dicom instance number!");
	this->_instance_number = strtoul(value.c_str(), NULL, 10);

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImagePositionPatient, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"ImagePositionPatient tag is missing in given dicom!");
	this->_image_position_patient = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_PixelSpacing, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"PixelSpacing tag is missing in given dicom!");
	this->_pixel_spacing = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImageOrientationPatient, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"ImageOrientationPatient tag is missing in given dicom!");
	this->_image_orientation = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_Rows, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not extract number of rows for given dicom!");
	this->_rows = strtoull(value.c_str(), NULL, 10);

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_Columns, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not extract number of columns for given dicom!");
	this->_columns = strtoull(value.c_str(), NULL, 10);

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_BitsAllocated, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not extract number of allocated bits for given dicom!");
	this->_allocated_bits = strtoul(value.c_str(), NULL, 10);

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PixelRepresentation, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not extract pixel representation for given dicom!");
	this->_is_signed_data = static_cast<unsigned short>(strtoul(value.c_str(), NULL, 10));

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PhotometricInterpretation, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not extract photometric interpretation for given dicom!");
	this->_photometric_interpretation = std::string(value.c_str());

	if (this->isColor())
	{
		if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PlanarConfiguration, value).good())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not extract planar configuration for given dicom!");
		this->_planar_configuration = static_cast<unsigned short>(strtoul(value.c_str(), NULL, 10));
	}
}

const unsigned char * tissuestack::imaging::DicomFileWrapper::getData()
{
	// extract some more info from the dicom image object
	std::unique_ptr<DicomImage> dcmTmp(new DicomImage(this->_file_name.c_str()));

	// the eventual data destination.
	// regardless of the bit depth or whether color we converge on 8 bit unsigned
	// because of our output choice
	unsigned long size = dcmTmp->getOutputDataSize(this->getAllocatedBits());
	std::unique_ptr<unsigned char[]> data_ptr(new unsigned char[size]);

	double min = 0;
	double max = pow(2, this->getAllocatedBits())-1;
	dcmTmp->getMinMaxValues(min,max);

	int two_power8 = static_cast<int>(pow(2, 8));
	int two_power_16 = static_cast<int>(pow(2, 16));

	// TODO: evaluate actual bit depth

	// 8 bit grey and 8 bit per channel color images should be easiest actually!
	if (this->isColor() || this->getAllocatedBits() == 8)
	{
		if (this->containsSignedData()) // deal with signed
		{
			std::unique_ptr<char> tmp_ptr(new char[size]);
			if (dcmTmp->getOutputData(tmp_ptr.get(), size, 8, 0, this->getPlanarConfiguration()) <= 0)
				return nullptr;

			// cast to unsigned
			for (unsigned long i=0;i<size;i++)
				data_ptr.get()[i] =
					static_cast<unsigned char>(tmp_ptr.get()[i]);
		} else if (dcmTmp->getOutputData(data_ptr.get(), size, 8, 0, this->getPlanarConfiguration()) <= 0)
			return nullptr;
	} else if (this->getAllocatedBits() == 16) // 16 bit data
	{
		std::unique_ptr<unsigned short[]> tmp_16bit_ptr(new unsigned short[size]);
		if (this->containsSignedData()) // deal with signed
		{
			std::unique_ptr<short[]> tmp_ptr(new short[size]);
			if (dcmTmp->getOutputData(tmp_ptr.get(), size, 16, 0, this->getPlanarConfiguration()) <= 0)
				return nullptr;

			// cast to unsigned
			for (unsigned long i=0;i<size;i++)
				tmp_16bit_ptr.get()[i] =
					static_cast<unsigned short>(tmp_ptr.get()[i]);
		} else if (dcmTmp->getOutputData(tmp_16bit_ptr.get(), size, 16, 0, this->getPlanarConfiguration()) <= 0)
			return nullptr;

		// turn 16 bit into 8 bit
		for (unsigned long i=0;i<size;i++)
			data_ptr.get()[i] = static_cast<unsigned char>((tmp_16bit_ptr.get()[i] * (two_power8-1)) / (two_power_16-1));
	}

	return data_ptr.release();
}


const std::string tissuestack::imaging::DicomFileWrapper::getFileName() const
{
	return this->_file_name;
}

const std::string tissuestack::imaging::DicomFileWrapper::getSeriesNumber() const
{
	return this->_series_number;
}

const unsigned long tissuestack::imaging::DicomFileWrapper::getInstanceNumber() const
{
	return this->_instance_number;
}

const std::string tissuestack::imaging::DicomFileWrapper::getImagePositionPatient() const
{
	return this->_image_position_patient;
}

const std::string tissuestack::imaging::DicomFileWrapper::getPixelSpacing() const
{
	return this->_pixel_spacing;
}

const std::string tissuestack::imaging::DicomFileWrapper::getImageOrientation() const
{
	return this->_image_orientation;
}

const unsigned long long int tissuestack::imaging::DicomFileWrapper::getHeight() const
{
	return this->_rows;
}

const unsigned long long int tissuestack::imaging::DicomFileWrapper::getWidth() const
{
	return this->_columns;
}

const unsigned long tissuestack::imaging::DicomFileWrapper::getAllocatedBits() const
{
	return this->_allocated_bits;
}

const bool tissuestack::imaging::DicomFileWrapper::containsSignedData() const
{
	return this->_is_signed_data == 0 ? false : true;
}

const unsigned short tissuestack::imaging::DicomFileWrapper::getPlanarConfiguration() const
{
	return this->_planar_configuration;
}


const bool tissuestack::imaging::DicomFileWrapper::isColor() const
{
	if (this->_photometric_interpretation.compare("RGB") == 0 ||
		this->_photometric_interpretation.compare("YBR_FULL") == 0 ||
		this->_photometric_interpretation.compare("YBR_FULL_422") == 0 ||
		this->_photometric_interpretation.compare("PALETTE COLOR") == 0)
		return true;

	return false;
}

tissuestack::imaging::DicomFileWrapper::~DicomFileWrapper()
{
	if (this->_isTemp)
		unlink(this->_file_name.c_str());
}
