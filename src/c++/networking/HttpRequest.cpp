#include "networking.h"

tissuestack::networking::HttpRequest::~HttpRequest() {
	// doing nothing at the moment
}

tissuestack::networking::HttpRequest::HttpRequest(const tissuestack::networking::RawHttpRequest * const raw_request) :
		tissuestack::networking::HttpRequest::HttpRequest(raw_request, false) {}

tissuestack::networking::HttpRequest::HttpRequest(const RawHttpRequest * const raw_request, const bool suppress_filter)
{
	if (!suppress_filter) {
		// first sanity check, then extract
		tissuestack::networking::HttpRequestSanityFilter preliminarySanityCheck;
		preliminarySanityCheck.applyFilter(raw_request);
	}

	// TODO: do your work
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
