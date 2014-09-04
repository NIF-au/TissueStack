#include "networking.h"
#include "imaging.h"
#include "services.h"

const std::string tissuestack::networking::TissueStackConversionRequest::SERVICE = "CONVERSION";


tissuestack::networking::TissueStackConversionRequest::~TissueStackConversionRequest() {}

tissuestack::networking::TissueStackConversionRequest::TissueStackConversionRequest(std::unordered_map<std::string, std::string> & request_parameters) {

	const std::string in_file =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "in_file");
	if (in_file.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'in_file' was not supplied!");

	this->_conversion =
		new tissuestack::services::TissueStackConversionTask(
				tissuestack::services::TissueStackTaskQueue::instance()->generateTaskId(),
			in_file,
			tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "out_file"));

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_CONVERSION);

}

const bool tissuestack::networking::TissueStackConversionRequest::isObsolete() const
{
	// we regard a conversion request 'obsolete' if there exists an output file by that very same name
	// or a running conversion task that works on that file already
	if (tissuestack::utils::System::fileExists(this->_conversion->getOutFile()))
		return true;

	return tissuestack::services::TissueStackTaskQueue::instance()->isBeingConverted(
		this->_conversion->getInputImageData()->getFileName());
}

const std::string tissuestack::networking::TissueStackConversionRequest::getContent() const
{
	return std::string("TS_CONVERSION");
}
