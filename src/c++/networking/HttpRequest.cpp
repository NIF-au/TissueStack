#include "networking.h"

tissuestack::networking::HttpRequest::~HttpRequest() {
	// doing nothing at the moment
}

tissuestack::networking::HttpRequest::HttpRequest(RawHttpRequest & raw_request)
{
	// first sanity check, then extract
	tissuestack::networking::HttpRequestSanityFilter preliminarySanityCheck;
	if (!preliminarySanityCheck.applyFilter(raw_request))
		throw tissuestack::common::TissueStackInvalidRequestException(std::string("request did not get past sanity check!"));
}

const std::string tissuestack::networking::HttpRequest::getHeader() const
{
	return std::string("NOT IMPLEMENTED YET");
}

const std::string tissuestack::networking::HttpRequest::dumpHeaders() const
{
	return std::string("NOT IMPLEMENTED YET");
}

const std::string tissuestack::networking::HttpRequest::getContent() const
{
	return this->_content;
}
