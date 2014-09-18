#include "networking.h"
#include "imaging.h"
#include "database.h"

tissuestack::imaging::TissueStackDataSet::TissueStackDataSet(
		const TissueStackImageData * image_data) : _image_data(image_data) {}

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

const tissuestack::imaging::TissueStackDataSet * tissuestack::imaging::TissueStackDataSet::fromDataBaseRecordWithId(
		const unsigned long long id,
		const bool includePlanes)
{
	if (id <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "DataSet Record Id has to greated than 0");

	// delegate
	return tissuestack::imaging::TissueStackDataSet::fromTissueStackImageData(
		tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(id, includePlanes));
}

void tissuestack::imaging::TissueStackDataSet::dumpDataSetContentIntoDebugLog() const
{
	this->_image_data->dumpImageDataIntoDebugLog();
}

void tissuestack::imaging::TissueStackDataSet::associateDataSets()
{
	if (!this->_image_data->hasNoAssociatedDataSets())
		const_cast<tissuestack::imaging::TissueStackImageData *>(this->_image_data)->clearAssociatedDataSets();
	tissuestack::database::DataSetDataProvider::findAssociatedDataSets(
		this->_image_data->getDataBaseId(), const_cast<tissuestack::imaging::TissueStackImageData *>(this->_image_data));
}

const std::string tissuestack::imaging::TissueStackDataSet::getDataSetId() const
{
	return this->_image_data->getFileName();
}

const tissuestack::imaging::TissueStackImageData * tissuestack::imaging::TissueStackDataSet::getImageData() const
{
	return this->_image_data;
}
