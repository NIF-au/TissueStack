#include "imaging.h"

tissuestack::imaging::TissueStackDataSet::TissueStackDataSet(
		const TissueStackImageData * image_data) : _image_data(image_data)
{
	//TODO: change status if we are being worked on
}

tissuestack::imaging::TissueStackDataSet::~TissueStackDataSet()
{
	if (this->_image_data)
		delete this->_image_data;
}

const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSet::fromFile(const std::string & filename)
{
	return tissuestack::imaging::TissueStackDataSet::fromTissueStackImageData(
			tissuestack::imaging::TissueStackImageData::fromFile(filename));
}

const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSet::fromTissueStackImageData(const TissueStackImageData * image_data)
{
	if (image_data == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException, "Cannot construct data set from null image data");

	return new tissuestack::imaging::TissueStackDataSet(image_data);
}

const tissuestack::imaging::DataSetStatus tissuestack::imaging::TissueStackDataSet::getStatus() const
{
	return this->_status;
}

const std::string tissuestack::imaging::TissueStackDataSet::getDataSetId() const
{
	return this->_image_data->getFileName();
}

const tissuestack::imaging::TissueStackImageData * tissuestack::imaging::TissueStackDataSet::getImageData() const
{
	return this->_image_data;
}
