#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackConversionTask::TissueStackConversionTask(
		const std::string id,
		const std::string input_file,
		const std::string output_file) : tissuestack::services::TissueStackTask(id, input_file)
{
	if (this->isInputDataRaw())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image does not need to converted to raw any more. It's already raw!");

	if (output_file.empty())
	{
		// we assamble a file name/location from the input file
		// by taking of the existing extension and appending .raw
		unsigned int pos = input_file.find_last_of(".");
		if (pos == std::string::npos)
			this->_output_file = input_file + ".raw";
		else
			this->_output_file =
				input_file.substr(0, pos) + ".raw";
	} else
	{
		// if the file does not end in .raw, we'll append it
		if (output_file.find(".raw") == std::string::npos)
			this->_output_file = output_file + ".raw";
		else
			this->_output_file = output_file;
	}

	// set total number of slices we have to work off
	unsigned long long int totalSlices = 0;
	for (auto d : this->getInputImageData()->getDimensionOrder())
		totalSlices += this->getInputImageData()->getDimension(d.at(0))->getNumberOfSlices();
	this->setTotalSlices(totalSlices);
}

tissuestack::services::TissueStackConversionTask::~TissueStackConversionTask() {}

const std::string tissuestack::services::TissueStackConversionTask::getOutFile() const
{
	return this->_output_file;
}

const tissuestack::services::TissueStackTaskType tissuestack::services::TissueStackConversionTask::getType() const
{
	return tissuestack::services::TissueStackTaskType::CONVERSION;
}

void tissuestack::services::TissueStackConversionTask::dumpTaskToDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug(
			"Conversion Task: %s => %s\n",
			this->getInputImageData()->getFileName().c_str(),
			this->_output_file.c_str());
}
