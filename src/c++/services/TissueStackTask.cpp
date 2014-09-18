#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackTask::TissueStackTask(
	const std::string id, const std::string input_file) :
		_id(id), _input_file(input_file), _status(tissuestack::services::TissueStackTaskStatus::QUEUED)
{
	// instantiate input file which will do all the necessary checks!
	this->_input_data = const_cast<tissuestack::imaging::TissueStackImageData *>(
		tissuestack::imaging::TissueStackImageData::fromFile(input_file));
}

tissuestack::services::TissueStackTask::~TissueStackTask()
{
	if (this->_input_data)
		delete this->_input_data;
}

const bool tissuestack::services::TissueStackTask::isInputDataRaw() const
{
	return this->_input_data->isRaw();
}

const tissuestack::imaging::TissueStackImageData * tissuestack::services::TissueStackTask::getInputImageData() const
{
	return this->_input_data;
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

void tissuestack::services::TissueStackTask::setTotalSlices(const unsigned long long int totalSlices)
{
	this->_total_slices = totalSlices;
}
