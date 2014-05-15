#include "networking.h"

tissuestack::networking::HttpdRequest::~HttpdRequest() {}

tissuestack::networking::HttpdRequest::HttpdRequest(std::string raw_content) :
	tissuestack::networking::HttpdRequest::HttpdRequest(tissuestack::networking::RawHttpdRequest(raw_content))
{
	throw tissuestack::common::TissueStackInvalidRequestException(std::string(" raw content not implemented "));
}

tissuestack::networking::HttpdRequest::HttpdRequest(tissuestack::networking::RawHttpdRequest raw_request)
{
	//do stuff
	throw tissuestack::common::TissueStackInvalidRequestException(std::string(" raw request not implemented "));
};

const std::string tissuestack::networking::HttpdRequest::getHeader() const
{
	return std::string("NULL");
}

const std::string tissuestack::networking::HttpdRequest::dumpHeaders() const
{
	return std::string("NULL");
}

const std::string tissuestack::networking::HttpdRequest::getBody() const
{
	return std::string("NULL");
}
