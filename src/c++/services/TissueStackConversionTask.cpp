#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackConversionTask::TissueStackConversionTask(
		const std::string id,
		const std::string input_file,
		const std::string output_file) : tissuestack::services::TissueStackTask(id, input_file), _output_file(output_file)
{
	if (this->isInputDataRaw())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Image does not need to converted to raw any more. It's already raw!");

	// TODO: set the output file if empty
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
 // TODO: implement
}
