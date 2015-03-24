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

	if (!image_data->isRaw() && image_data->getFormat() != tissuestack::imaging::FORMAT::DATABASE)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException, "Cannot construct data set from image data that is not raw or database!");

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
