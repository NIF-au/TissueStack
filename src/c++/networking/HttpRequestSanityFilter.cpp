#include "networking.h"

tissuestack::networking::HttpRequestSanityFilter::~HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

tissuestack::networking::HttpRequestSanityFilter::HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

const tissuestack::common::Request * const tissuestack::networking::HttpRequestSanityFilter::applyFilter(const tissuestack::common::Request * const request) const
{
	if (request == nullptr)   THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "applyFilter was called with NULL");

	if (request->getType() != tissuestack::common::Request::Type::RAW_HTTP_REQUEST)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with non raw request");

	// quick check if we begin the right way ...
	const std::string * const raw_content = request->getContent();
	// there need to be 14 characters at a bare minimum, e.g. GET / HTTP/X.X
	// of course, there better be more but we check that later
	if (raw_content == nullptr || raw_content->length() < 10)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with incomplete http request");

	// for tissuestack we are not interested in any NON GET type of requests
	if (raw_content->find("GET") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Tissue Stack only wants to deal with GET requests");

	// we are good => return a proper HttpRequest instance
	return new tissuestack::networking::HttpRequest(static_cast<const tissuestack::networking::RawHttpRequest * const>(request), true);;
};
