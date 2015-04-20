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

const bool tissuestack::imaging::TissueStackNiftiData::isRaw() const
{
	return false;
}

const bool tissuestack::imaging::TissueStackNiftiData::isColor() const
{
	return this->_is_color;
}

const nifti_image * tissuestack::imaging::TissueStackNiftiData::getNiftiHandle() const
{
	return this->_volume;
}

const double tissuestack::imaging::TissueStackNiftiData::getMin() const
{
	return this->_min;
}
const double tissuestack::imaging::TissueStackNiftiData::getMax() const
{
	return this->_max;
}


tissuestack::imaging::TissueStackNiftiData::TissueStackNiftiData(const std::string & filename) :
		tissuestack::imaging::TissueStackImageData(filename, tissuestack::imaging::FORMAT::NIFTI)
{
	// open file
	this->_volume = nifti_image_read(filename.c_str(), 0);
	if (this->_volume == nullptr)
		THROW_TS_EXCEPTION(
			tissuestack::common::TissueStackApplicationException,
			"Failed to open supposed NIFTI file!");

	// extract dimension number
	int numberOfDimensions = this->_volume->ndim;
	// in these 2 cases we are undoubtedly color
	if (this->_volume->datatype == NIFTI_TYPE_RGB24
		|| this->_volume->datatype == NIFTI_TYPE_RGBA32)
		this->_is_color = true;

	if (numberOfDimensions > 3) { // this is how we handle time series data and potential RGB, data types in headers are not reliable
		if (numberOfDimensions > 4) this->_is_color = true; // if there is yet another dimension after time we assume RGB
		numberOfDimensions = 3; // internally we use this to determine our loop length
	}

	if (numberOfDimensions <=0)
		THROW_TS_EXCEPTION(
			tissuestack::common::TissueStackApplicationException,
			"Nifi file has zero dimensions!");

	// get the volume dimensions
	unsigned short i = 0;
	unsigned int width2D = 0;
	while (i < numberOfDimensions)
	{
		if (i > 2) // this is for non spatial, higher dimensions
			break;

		// we have to generate a name
		std::string name("xspace");
		name[0] = 'x' + i;

		// create our dimension object and add it
		if ((numberOfDimensions == 2 && i == 0) || numberOfDimensions > 2)
		{
			width2D = static_cast<unsigned int>(this->_volume->dim[i+1]);
			this->addDimension(
				new tissuestack::imaging::TissueStackDataDimension(
					name,
					0, // bogus offset for NIFTI
					static_cast<unsigned long long int>(width2D),
					0)); // bogus slice size for NIFTI
		} else if (numberOfDimensions == 2 && i ==1)
		{
			unsigned long long int height2D =
					static_cast<unsigned long long int>(this->_volume->dim[i+1]);
			tissuestack::imaging::TissueStackDataDimension * d =
				new tissuestack::imaging::TissueStackDataDimension(
					name,
					0, // bogus offset for NIFTI
					1,
					height2D * width2D);
			d->setWidthAndHeight(width2D, height2D, width2D, height2D);
			this->addDimension(d);
		}

		// add start and step
		this->addCoordinate(static_cast<float>(this->_volume->sto_xyz.m[i][3]));
		this->addStep(static_cast<float>(this->_volume->pixdim[i + 1]));
		i++;
	}

	// further dimension info initialization (order of function calls matter!)
	if (numberOfDimensions > 2)
		this->initializeDimensions(true);
	this->detectAndCorrectFor2DData();
	this->generateRawHeader();
	this->initializeOffsetsForNonRawFiles();
	this->setGlobalMinMax();
}

tissuestack::imaging::TissueStackNiftiData::~TissueStackNiftiData()
{
	if (this->_volume) free(this->_volume);
}

void tissuestack::imaging::TissueStackNiftiData::setGlobalMinMax()
{
	if (this->_is_color
			|| this->_volume->datatype == NIFTI_TYPE_UINT8
			|| this->_volume->datatype == NIFTI_TYPE_RGB24
			|| this->_volume->datatype == NIFTI_TYPE_RGBA32
			|| this->_volume->datatype == NIFTI_TYPE_COMPLEX64
			|| this->_volume->datatype == NIFTI_TYPE_COMPLEX128
			|| this->_volume->datatype == NIFTI_TYPE_COMPLEX256
			|| this->_volume->datatype == DT_BINARY
			|| this->_volume->datatype == 0)
	{
		// these types are either not supported or:
		// don't require intensity value adjustments as they are already within the 8bit unsigned range
		this->_min = 0;
		this->_max = 255;
		return;
	}

	int dims[8] = { 0, -1, -1, -1, -1, -1, -1, -1 };
	// set time slice to 0 for any data set with dimensionality greater than 3
	if (this->_volume->ndim > 3) dims[4] = 0;

	unsigned int slice = 0;
	void *data_in = NULL, *in = NULL;

	const tissuestack::imaging::TissueStackDataDimension * firstDim =
		this->get2DDimension() != nullptr ?
				this->get2DDimension() :
			this->getDimensionByOrderIndex(0);
	if (firstDim == nullptr)
		THROW_TS_EXCEPTION(
			tissuestack::common::TissueStackApplicationException,
			"Could not find first dimension of read NIFTI file!");
	short ind = this->getIndexForPlane(firstDim->getName()[0]);

	unsigned long long int size_per_slice =
		firstDim->getSliceSize();
	unsigned long long int expected_bytes =
		size_per_slice * static_cast<unsigned long long int>(this->_volume->nbyper);

	// we use the first dimension, why not, don't make a difference to me ...
	while (slice < firstDim->getNumberOfSlices())  // SLICE LOOP
	{
		dims[1+ind] = slice;

		int ret = nifti_read_collapsed_image(this->_volume, dims, &data_in);
		if (ret < 0)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"Failed to read NIFTI file!");

		//sanity check
		if (static_cast<unsigned long long int>(ret) != expected_bytes)
			THROW_TS_EXCEPTION(
				tissuestack::common::TissueStackApplicationException,
				"NIFTI read: number of read bytes does not match expected bytes!");

		for (unsigned int i = 0; i < size_per_slice; i++) {
			// move start back "data type" number of bytes...
			if (i == 0)
				in = data_in;
			else
				in = (void *)(((char*) in) + this->_volume->nbyper);

			switch (this->_volume->datatype) {
				case NIFTI_TYPE_INT8: // signed char
					if (((char *) in)[0] < this->_min) this->_min = ((char *) in)[0];
					if (((char *) in)[0] > this->_min) this->_max = ((char *) in)[0];
					break;
				case NIFTI_TYPE_UINT16: // unsigned short
					if (((unsigned short *) in)[0] < this->_min) this->_min = ((unsigned short *) in)[0];
					if (((unsigned short *) in)[0] > this->_max) this->_max = ((unsigned short *) in)[0];
					break;
				case NIFTI_TYPE_UINT32: // unsigned int
					if (((unsigned int *) in)[0] < this->_min) this->_min = ((unsigned int *) in)[0];
					if (((unsigned int *) in)[0] > this->_max) this->_max = ((unsigned int *) in)[0];
					break;
				case NIFTI_TYPE_INT16: // signed short
					if (((short *) in)[0] < this->_min) this->_min = ((short *) in)[0];
					if (((short *) in)[0] > this->_max) this->_max = ((short *) in)[0];
					break;
				case NIFTI_TYPE_INT32: // signed int
					if (((int *) in)[0] < this->_min) this->_min = ((int *) in)[0];
					if (((int *) in)[0] > this->_max) this->_max = ((int *) in)[0];
					break;
				case NIFTI_TYPE_UINT64: // unsigned long long
					if (((unsigned long long int *) in)[0] < this->_min) this->_min = ((unsigned long long int *) in)[0];
					if (((unsigned long long int *) in)[0] > this->_max) this->_max = ((unsigned long long int *) in)[0];
					break;
				case NIFTI_TYPE_INT64: // signed long long
					if (((long long int *) in)[0] < this->_min) this->_min = ((long long int *) in)[0];
					if (((long long int *) in)[0] > this->_max) this->_max = ((long long int *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT32: //	float
					if (((float *) in)[0] < this->_min) this->_min = ((float *) in)[0];
					if (((float *) in)[0] > this->_max) this->_max = ((float *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT64: //	double
					if (((double *) in)[0] < this->_min) this->_min = ((double *) in)[0];
					if (((double *) in)[0] > this->_max) this->_max = ((double *) in)[0];
					break;
				case NIFTI_TYPE_FLOAT128: // long double
					if (((long double *) in)[0] < this->_min) this->_min = ((long double *) in)[0];
					if (((long double *) in)[0] > this->_max) this->_max = ((long double *) in)[0];
					break;
			}
		}

		// increment slice
		slice++;
		if (data_in != NULL)
		{
			free(data_in);
			data_in = NULL;
		}
	}
}
