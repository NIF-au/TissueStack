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
#include "services.h"

tissuestack::services::TissueStackTask::TissueStackTask(
	const std::string id, const std::string input_file) :
		_id(id), _input_file(input_file), _status(tissuestack::services::TissueStackTaskStatus::QUEUED)
{
	// we check if we have a zipped source file
	// if so switch status to unzipping and instantiate image data later to
	// enable an instant return as opposed to waiting till the end of unzipping
	this->checkWhetherZipFile(input_file);

	if (!this->hasBeenUnzipped())
	{
		this->setStatus(tissuestack::services::TissueStackTaskStatus::UNZIPPING);
		return;
	}

	this->readImageData();
}

void tissuestack::services::TissueStackTask::readImageData()
{
	// instantiate input file which will do all the necessary checks!
	this->_input_data = const_cast<tissuestack::imaging::TissueStackImageData *>(
		tissuestack::imaging::TissueStackImageData::fromFile(this->_input_file));
	this->_has_been_Unzipped = true; // if we came here for zipped files, we know unzipped worked
}


tissuestack::services::TissueStackTask::~TissueStackTask()
{
	if (this->_input_data)
		delete this->_input_data;
}

const bool tissuestack::services::TissueStackTask::isInputDataRaw() const
{
	if (!this->hasBeenUnzipped() && this->_input_data == nullptr)
		return false;

	return this->_input_data->isRaw();
}

const tissuestack::imaging::TissueStackImageData * tissuestack::services::TissueStackTask::getInputImageData() const
{
	return this->_input_data;
}

void tissuestack::services::TissueStackTask::checkWhetherZipFile(const std::string filename)
{
	// a very superficial extension check since we'll discover zip file problems later anyhow
	std::string input_file_lower_case = filename;
	std::transform(input_file_lower_case.begin(), input_file_lower_case.end(), input_file_lower_case.begin(), tolower);
	if (input_file_lower_case.length() > 4 &&
		input_file_lower_case.substr(input_file_lower_case.length()-4).compare(".zip") == 0)
		this->_is_zip_file = true;
}

const bool tissuestack::services::TissueStackTask::hasBeenUnzipped() const
{
	if (!this->_is_zip_file)
		return true;

	return this->_has_been_Unzipped;
}

const std::string tissuestack::services::TissueStackTask::getId() const
{
	return this->_id;
}

const float tissuestack::services::TissueStackTask::getProgress() const
{
	if (this->_total_slices == 0)
		return 0;

	return (static_cast<float>(this->_slices_done) /
		static_cast<float>(this->_total_slices)) * static_cast<float>(100);
}

const tissuestack::services::TissueStackTaskStatus tissuestack::services::TissueStackTask::getStatus() const
{
	return this->_status;
}

const unsigned long long int tissuestack::services::TissueStackTask::getSlicesDone() const
{
	return this->_slices_done;
}

const unsigned long long int tissuestack::services::TissueStackTask::getTotalSlices() const
{
	return this->_total_slices;
}

void tissuestack::services::TissueStackTask::setStatus(const tissuestack::services::TissueStackTaskStatus status)
{
	this->_status = status;
}


void tissuestack::services::TissueStackTask::setSlicesDone(const unsigned long long int slicesDone)
{
	this->_slices_done = slicesDone;
}

const bool tissuestack::services::TissueStackTask::incrementSlicesDone()
{
	++this->_slices_done;
	if (this->_slices_done >= this->_total_slices)
		return true;

	return false;
}

const std::string tissuestack::services::TissueStackTask::getInputFileName() const
{
	return this->_input_file;
}

void tissuestack::services::TissueStackTask::setTotalSlices(const unsigned long long int totalSlices)
{
	this->_total_slices = totalSlices;
}
