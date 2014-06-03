#include "networking.h"

const std::string tissuestack::networking::TissueStackImageRequest::SERVICE = "IMAGE";


tissuestack::networking::TissueStackImageRequest::~TissueStackImageRequest() {}

tissuestack::networking::TissueStackImageRequest::TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	// TODO: we check and assign to the according members
	THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "NOT IMPLEMENTED YET!");

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_IMAGE);

}

const bool tissuestack::networking::TissueStackImageRequest::isObsolete() const
{
	// use optional session id and timestamp against timestamp request store...
	if (this->_request_id == 0 || this->_request_timeout == 0) return false;

	return tissuestack::common::RequestTimeStampStore::instance()->checkForExpiredEntry(this->_request_id, this->_request_timeout);
}

const std::string tissuestack::networking::TissueStackImageRequest::getContent() const
{
	return std::string("TS_IMAGE");
}
