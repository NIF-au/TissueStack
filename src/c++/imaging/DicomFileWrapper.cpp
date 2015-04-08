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
	#include "dcmtk/dcmimage/diregist.h"
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
	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, value).good())
		if (!dicomFormat.getDataset()->findAndGetOFString(DCM_SeriesNumber, value).good())
			this->_series_number = "0";
		else
			this->_series_number = std::string(value.c_str());
	else
		this->_series_number = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_InstanceNumber, value).good())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not read dicom instance number!");
	this->_instance_number = strtoul(value.c_str(), NULL, 10);

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImagePositionPatient, value).good())
		this->_image_position_patient = "0\\0\\0";
	else
		this->_image_position_patient = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_PixelSpacing, value).good())
		this->_pixel_spacing = "1\\1";
	else
		this->_pixel_spacing = std::string(value.c_str());

	if (!dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImageOrientationPatient, value).good())
		this->_image_orientation = "1\\0\\0\\0\\1\\0";
	else
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
		this->_is_signed_data = 0;
	else
		this->_is_signed_data = static_cast<unsigned short>(strtoul(value.c_str(), NULL, 10));

	if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PhotometricInterpretation, value).good())
		this->_photometric_interpretation = "MONOCHROME2";
	else
		this->_photometric_interpretation = std::string(value.c_str());

	if (dicomFormat.getDataset()->findAndGetOFString(DCM_PixelPaddingValue, value).good())
		this->_pixel_padding_value = atoi(value.c_str());

	if (this->isColor())
	{
		if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PlanarConfiguration, value).good())
			this->_planar_configuration = 0;
		else
			this->_planar_configuration = static_cast<unsigned short>(strtoul(value.c_str(), NULL, 10));
	}
}

const unsigned char * tissuestack::imaging::DicomFileWrapper::getData()
{
	// the dicom image
	std::unique_ptr<DicomImage> dcmTmp(new DicomImage(this->_file_name.c_str(), CIF_AcrNemaCompatibility));

	unsigned long data_size = dcmTmp->getOutputDataSize(
		(this->getAllocatedBits() <= 8 || this->isColor()) ? 8 : 16);
	unsigned long image_size = this->getWidth() * this->getHeight();
	if (data_size == 0)
		data_size = image_size * (this->getAllocatedBits() / 2);
	if (image_size == 0)
		image_size = data_size;
	const unsigned long finished_image_size = image_size * 3;

	// we converge on an RGB format 8 bit per channel which is what our raw format is
	std::unique_ptr<unsigned char[]> data_ptr(new unsigned char[finished_image_size]);

	double min = pow(2, this->getAllocatedBits())-1;
	double max = 0;
	dcmTmp->getMinMaxValues(min,max);

	int two_power8 = static_cast<int>(pow(2, 8));
	int two_power_16 = static_cast<int>(pow(2, 16));

	if (this->isColor()) // COLOR
	{
		if (dcmTmp->getOutputData(data_ptr.get(), data_size, 8, 0, 0) <= 0)
			return nullptr;

		return data_ptr.release();
	}

	if (this->getAllocatedBits() <= 8) // 8 BIT
	{
		if (this->containsSignedData()) // SIGNED
		{
			std::unique_ptr<char[]> tmp_ptr(new char[image_size]);
			if (dcmTmp->getOutputData(tmp_ptr.get(), data_size, 8, 0, 0) <= 0)
				return nullptr;

			// convert to unsigned
			min = pow(2, this->getAllocatedBits())-1;
			max = 0;
			for (unsigned long i=0;i<image_size;i++)
			{
				const unsigned long rgbOffset = i * 3;
				const unsigned char val =
					static_cast<unsigned char>(static_cast<int>(tmp_ptr.get()[i]) + (two_power8 / 2));
				data_ptr.get()[rgbOffset] = data_ptr.get()[rgbOffset+1] = data_ptr.get()[rgbOffset+2] = val;

				if (val < min)
					min = val;
				if (val > max)
					max = val;
			}

			// now that we have min and max ... fit to contrast range linearly
			for (unsigned long i=0;i<finished_image_size;i++)
				data_ptr.get()[i] =
					static_cast<unsigned char>(((static_cast<double>(data_ptr.get()[i])-min) * (two_power8-1)) / (max-min));

			// return
			return data_ptr.release();
		}

		// UNSIGNED
		std::unique_ptr<unsigned char[]> tmp_ptr(new unsigned char[image_size]);
		if (dcmTmp->getOutputData(tmp_ptr.get(), data_size, 8, 0, 0) <= 0)
			return nullptr;

		// make RGB data
		for (unsigned long i=0;i<image_size;i++)
		{
			const unsigned long rgbOffset = i * 3;
			data_ptr.get()[rgbOffset] =
			data_ptr.get()[rgbOffset+1] =
			data_ptr.get()[rgbOffset+2] = tmp_ptr.get()[i];
		}

		return data_ptr.release();
	}

	if (this->getAllocatedBits() > 8) // 12/16 BITS
	{
		std::unique_ptr<unsigned short[]> tmp_16bit_ptr(new unsigned short[finished_image_size]);

		if (this->containsSignedData()) // SIGNED
		{
			std::unique_ptr<short[]> tmp_ptr(new short[image_size]);
			if (dcmTmp->getOutputData(tmp_ptr.get(), data_size, 16, 0, 0) <= 0)
				return nullptr;

			// convert to unsigned
			min = pow(2, this->getAllocatedBits())-1;
			max = 0;
			for (unsigned long i=0;i<image_size;i++)
			{
				const unsigned long rgbOffset = i * 3;
				const unsigned short val =
					static_cast<unsigned short>(static_cast<int>(tmp_ptr.get()[i]) + (two_power_16 / 2));
				tmp_16bit_ptr.get()[rgbOffset] = tmp_16bit_ptr.get()[rgbOffset+1] = tmp_16bit_ptr.get()[rgbOffset+2] = val;

				if (val < min)
					min = val;
				if (val > max)
					max = val;
			}

			// now that we have min and max ... fit to contrast range linearly to make it 8 bit
			for (unsigned long i=0;i<finished_image_size;i++)
				data_ptr.get()[i] =
					static_cast<unsigned char>(((static_cast<double>(tmp_16bit_ptr.get()[i])-min) * (two_power8-1)) / (max-min));

			return data_ptr.release();
		}

		// UNSIGNED
		if (dcmTmp->getOutputData(tmp_16bit_ptr.get(), data_size, 16, 0, 0) <= 0)
			return nullptr;

		// turn 16 bit into 8 bit
		for (unsigned long i=0;i<image_size;i++)
		{
			const unsigned long rgbOffset = i * 3;
			data_ptr.get()[rgbOffset] = data_ptr.get()[rgbOffset+1] = data_ptr.get()[rgbOffset+2] =
				static_cast<unsigned char>((static_cast<double>(tmp_16bit_ptr.get()[i]) * (two_power8-1)) / (two_power_16-1));
		}
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

const int tissuestack::imaging::DicomFileWrapper::getPixelPadddingValue() const
{
	return this->_pixel_padding_value;
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
