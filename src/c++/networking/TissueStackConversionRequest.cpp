#include "networking.h"
#include "imaging.h"
#include "services.h"

const std::string tissuestack::networking::TissueStackConversionRequest::SERVICE = "CONVERSION";


tissuestack::networking::TissueStackConversionRequest::~TissueStackConversionRequest()
{
	if (this->_conversion)
		delete this->_conversion;
}

tissuestack::networking::TissueStackConversionRequest::TissueStackConversionRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	// we need a valid session
	if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "session")))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"Invalid Session! Please Log In.");

	const std::string in_file =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "file");
	if (in_file.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'file' was not supplied!");

	this->_conversion =
		new tissuestack::services::TissueStackConversionTask(
				tissuestack::services::TissueStackTaskQueue::instance()->generateTaskId(),
			in_file,
			tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "new_raw_file"));

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_CONVERSION);

}

const tissuestack::services::TissueStackConversionTask *
	tissuestack::networking::TissueStackConversionRequest::getTask(
			const bool nullOutPointer)
{
	if (!nullOutPointer)
		return this->_conversion;

	const tissuestack::services::TissueStackConversionTask * cpy = this->_conversion;
	this->_conversion = nullptr;

	return cpy;
}

const bool tissuestack::networking::TissueStackConversionRequest::isObsolete() const
{
	// we regard a conversion request 'obsolete'
	//if there exists a running conversion task that works on that file
	return tissuestack::services::TissueStackTaskQueue::instance()->isBeingConverted(
		this->_conversion->getInputImageData()->getFileName());
}

const std::string tissuestack::networking::TissueStackConversionRequest::getContent() const
{
	return std::string("TS_CONVERSION");
}
