#include "networking.h"
#include "imaging.h"
#include "services.h"

const std::string tissuestack::networking::TissueStackPreTilingRequest::SERVICE = "TILE";

tissuestack::networking::TissueStackPreTilingRequest::~TissueStackPreTilingRequest() {}

tissuestack::networking::TissueStackPreTilingRequest::TissueStackPreTilingRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	const std::string in_file =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "in_file");
	if (in_file.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'in_file' was not supplied!");

	//TODO: gather all the params from the request
	this->_tiling =
		new tissuestack::services::TissueStackTilingTask(
				tissuestack::services::TissueStackTaskQueue::instance()->generateTaskId(),
			in_file,
			"");

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_TILING);

}

const bool tissuestack::networking::TissueStackPreTilingRequest::isObsolete() const
{
	// a tiling request is regarded obsolete if there is another one running for the same raw file!
	return tissuestack::services::TissueStackTaskQueue::instance()->isBeingTiled(
		this->_tiling->getInputImageData()->getFileName());
}

const std::string tissuestack::networking::TissueStackPreTilingRequest::getContent() const
{
	return std::string("TS_TILING");
}
