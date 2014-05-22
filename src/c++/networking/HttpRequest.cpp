#include "networking.h"

tissuestack::networking::HttpRequest::~HttpRequest() {}

tissuestack::networking::HttpRequest::HttpRequest(const tissuestack::networking::RawHttpRequest * const raw_request) :
		tissuestack::networking::HttpRequest::HttpRequest(raw_request, false) {
	// we merely delegate
}

tissuestack::networking::HttpRequest::HttpRequest(const RawHttpRequest * const raw_request, const bool suppress_filter)
{
	if (raw_request == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "HttpRequest cannot be instantiated with NULL");

	// this ensures that we are only sanity checked once !
	if (!suppress_filter) {
		// first sanity check, then extract
		tissuestack::networking::HttpRequestSanityFilter preliminarySanityCheck;
		preliminarySanityCheck.applyFilter(raw_request);
	}

	// we go on to dissect the GET request, we really don't care for any other http method
	const std::string * const raw_content = raw_request->getContent();
	int nPos = raw_content->find(' ', 4);
	// if we are under 3, there is something wrong, the URI needs to start at position 3 => 'GET /somequerystring'
	if (nPos < 4)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest with malformed GET");

	// now cut out query string
	this->_query_string = raw_content->substr(4,nPos-4);
	std::cout << "In between: *" << this->_query_string.c_str() << "*" <<  std::endl;

	// find start of actual query string and prune anything up to and including ?
	nPos = this->_query_string.find('?');
	if (nPos < 0) // bad: we don't have a query string
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest without query string!");
	this->_query_string.replace(0, nPos+1, "");

	// go over query string and stuff every parameter into the map!
	int lengthOfQueryString = this->_query_string.length();
	int cursor = nPos = 0;
	std::string key("");
	while (cursor < lengthOfQueryString)
	{
		// TODO: eliminate bugs and run regex over it to eliminate &=
		// read up to = or &
		if (this->_query_string[cursor] == '=') // remember key unless we have the next &
		{
			if ((cursor+1) < lengthOfQueryString && this->_query_string[cursor+1] == '&') // skip & right after ?
			{
				cursor++;
				nPos++;
				continue;
			}
			if (nPos < cursor) // store only if we have a non empty key value
				key = this->_query_string.substr(nPos+1, cursor-(nPos+1));
			if ((cursor+1) <= lengthOfQueryString) nPos = cursor;
		}
		else if (this->_query_string[cursor] == '&'
					|| cursor+1 == lengthOfQueryString) // new key or empty value ?
		{
			if ((cursor+1) < lengthOfQueryString && this->_query_string[cursor+1] == '=') // skip = right after &
			{
				cursor++;
				nPos++;
				continue;
			}

			if (key.length() > 0) // we have a value => uri decode and store it
			{
				// TODO:uri decode
				if (nPos < cursor) // store only if we have a non empty value
					this->_parameters[key] = this->_query_string.substr(nPos+1, cursor-(nPos+1));
				// reset key
				key.empty();
			}
			if ((cursor+1) <= lengthOfQueryString) nPos = cursor;
		}
		// increment cursor
		cursor++;
	}

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::HTTP_REQUEST);

}

const std::string tissuestack::networking::HttpRequest::getParameter(std::string name) const
{
	try
	{
		return this->_parameters.at(name);
	} catch (const std::out_of_range& ignored) { }
	return std::string("");
}

const std::string tissuestack::networking::HttpRequest::dumpParameters() const
{
	std::ostringstream in;
	for (auto s : this->_parameters)
		in << "*" << s.first << "*" << " => *" << s.second << "*" << std::endl;
	return in.str();
}

const std::string * const tissuestack::networking::HttpRequest::getContent() const
{
	return &this->_query_string;
}
