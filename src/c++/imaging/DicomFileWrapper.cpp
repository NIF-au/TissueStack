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

#define CSA_ImageHeaderInfo DcmTagKey(0x0029,0x1010)
#define CSA_ImageSeriesHeaderInfo DcmTagKey(0x0029,0x1020)
#define CSA_NumberOfImagesInMosaic DcmTagKey(0x0019,0x100a)

#define ASCCONV_BEGIN "### ASCCONV BEGIN ###"
#define ASCCONV_END "### ASCCONV END ###"
#define LSIZE "sSliceArray.lSize"

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

	OFString value;
	if (dicomFormat.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, value).good())
	{
		std::string transferSyntaxUID = value.c_str();
		std::transform(transferSyntaxUID.begin(), transferSyntaxUID.end(), transferSyntaxUID.begin(), toupper);
		if (transferSyntaxUID.find("JPEG2000") != std::string::npos)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"We don't support JPEG2000!");
	}

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

	if (dicomFormat.getDataset()->findAndGetOFString(DCM_MRAcquisitionType, value).good()) // are we 3D
		this->_acquisitionType = std::string(value.c_str());

	if (dicomFormat.getDataset()->findAndGetOFString(DCM_PatientPosition, value).good())
		this->_patient_position = std::string(value.c_str());

	if (dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImageType, value).good()) // are we a mosaic
	{
		std::string v = std::string(value.c_str());
		std::transform(v.begin(), v.end(), v.begin(), toupper);
		if (v.find("MOSAIC") != std::string::npos) // have a look for numbers of images in mosaic
		{
			if (dicomFormat.getDataset()->findAndGetOFString(CSA_NumberOfImagesInMosaic, value).good())
				this->_number_of_images_in_mosaic = strtoul(value.c_str(), NULL, 10);

			DcmElement * csaImageHeaderInfo =
				const_cast<DcmElement *>(
						this->findDcmElement(dicomFormat.getDataset(), CSA_ImageSeriesHeaderInfo));
			if (csaImageHeaderInfo != nullptr)
			{
				Uint8 * csaImageHeaderContent = nullptr;
				unsigned long int length =
					csaImageHeaderInfo->getLength();
				status = csaImageHeaderInfo->getUint8Array(csaImageHeaderContent);
				if (status.good())
				{
					std::string tmp = std::string((const char *) csaImageHeaderContent, length);
					size_t ascconf_start = tmp.find(ASCCONV_BEGIN);
					if (ascconf_start == std::string::npos) // plan B: try other header
					{
						csaImageHeaderInfo =
							const_cast<DcmElement *>(
								this->findDcmElement(dicomFormat.getDataset(), CSA_ImageHeaderInfo));
						if (csaImageHeaderInfo != nullptr)
						{
							csaImageHeaderContent = nullptr;
							length = csaImageHeaderInfo->getLength();
							status = csaImageHeaderInfo->getUint8Array(csaImageHeaderContent);
							tmp = std::string((const char *) csaImageHeaderContent, length);
							ascconf_start = tmp.find(ASCCONV_BEGIN); // try again with backup header
						}
					}
					if (ascconf_start != std::string::npos)
					{
						ascconf_start += strlen(ASCCONV_BEGIN) + 1;
						size_t ascconf_end = tmp.find(ASCCONV_END) - 1;
						if (ascconf_end != std::string::npos)
							this->_ascconv = tmp.substr(ascconf_start, ascconf_end-ascconf_start);
					}
				}
			}

			if (this->_number_of_images_in_mosaic == 0 &&
				!this->_ascconv.empty()) // plan B for number of images in mosaic
			{
				size_t lSize_start = this->_ascconv.find(LSIZE);
				if (lSize_start != std::string::npos)
				{
					lSize_start = this->_ascconv.find("=", lSize_start);
					if (lSize_start != std::string::npos)
					{
						lSize_start++;
						size_t lSize_end = this->_ascconv.find("\n", lSize_start);
						if (lSize_end != std::string::npos)
						{
							std::string lSizeString = this->_ascconv.substr(lSize_start, lSize_start-lSize_end);
							this->_number_of_images_in_mosaic = strtoul(lSizeString.c_str(), NULL, 10);
						}
					}
				}
			}
		}

	}

	if (dicomFormat.getDataset()->findAndGetOFStringArray(DCM_ImagesInAcquisition, value).good())
		this->_number_of_images_in_series_or_acquisition = strtoul(value.c_str(), NULL, 10);

	if (this->_number_of_images_in_series_or_acquisition == 0 &&
		dicomFormat.getDataset()->findAndGetOFStringArray(DCM_RETIRED_ImagesInSeries, value).good())
		this->_number_of_images_in_series_or_acquisition = strtoul(value.c_str(), NULL, 10);

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

	if (this->isColor())
	{
		if (!dicomFormat.getDataset()->findAndGetOFString(DCM_PlanarConfiguration, value).good())
			this->_planar_configuration = 0;
		else
			this->_planar_configuration = static_cast<unsigned short>(strtoul(value.c_str(), NULL, 10));
	}
}

const DcmElement * tissuestack::imaging::DicomFileWrapper::findDcmElement(
	const DcmDataset * dataSet, const DcmTagKey & tagKey) const
{
	DcmStack stack;
	OFCondition status = const_cast<DcmDataset *>(dataSet)->nextObject(stack, OFTrue);
	while (status.good())
	{
		const  DcmObject * dobject = stack.top();
		const DcmElement * delem = (DcmElement *) dobject;
		if (delem->getTag().getXTag() == tagKey)
			return delem;

		status = const_cast<DcmDataset *>(dataSet)->nextObject(stack, OFTrue);
	}

	return nullptr;
}

const unsigned long int tissuestack::imaging::DicomFileWrapper::getNumberOfImagesInSeriesOrAcquision() const
{
	return this->_number_of_images_in_series_or_acquisition;
}

const bool tissuestack::imaging::DicomFileWrapper::isMosaic() const
{
	return (this->_number_of_images_in_mosaic != 0);
}

const std::string tissuestack::imaging::DicomFileWrapper::getAcquisitionType() const
{
	return this->_acquisitionType;
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

const std::string tissuestack::imaging::DicomFileWrapper::getPatientPosition() const
{
	return this->_patient_position;
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
