#include "networking.h"

std::unordered_map<std::string, std::string> tissuestack::networking::HttpRequest::MinimalURIDecodingTable
{
		{"%20", " "}, // blank
		{"%26", "&"}, // &
		{"%25", "%"}, // %
		{"%3D", "="}, // =
		{"%2B", "+"}, // +
		{"%2F", "/"}, // /
		{"%23", "#"}, // #
		{"%2D", "-"}, // -
		{"%5F", "_"} // _
};

tissuestack::networking::HttpRequest::~HttpRequest() {}

tissuestack::networking::HttpRequest::HttpRequest(const tissuestack::networking::RawHttpRequest * const raw_request) :
		tissuestack::networking::HttpRequest::HttpRequest(raw_request, false) {
	// we merely delegate
}

tissuestack::networking::HttpRequest::HttpRequest(const RawHttpRequest * const raw_request, const bool suppress_filter) : _query_string("")
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
	const std::string raw_content = raw_request->getContent();
	int nPos = raw_content.find(' ', 4);
	// if we are under 3, there is something wrong, the URI needs to start at position 3 => 'GET /somequerystring'
	if (nPos < 4)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest with malformed GET");

	// now cut out query string
	this->_query_string = raw_content.substr(4,nPos-4);

	// find start of actual query string and prune anything up to and including ?
	nPos = this->_query_string.find('?');
	if (nPos < 0) // bad: we don't have a query string
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest without query string!");
	this->_query_string.replace(0, nPos+1, "");

	// parse query string and stuff every parameter into the map!
	this->processsQueryString();

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::HTTP);

}

void tissuestack::networking::HttpRequest::processsQueryString()
{
	int lengthOfQueryString = 0;
	lengthOfQueryString = this->_query_string.length();
	int cursor = 0, nPos = 0;
	std::string key = "";
	while (cursor < lengthOfQueryString)
	{
		if (this->_query_string[cursor] == '=') // we have a key or value (potentially)
		{
			// skipping next &/=
			if (this->skipNextCharacterCheck(lengthOfQueryString, cursor, nPos, key))
				continue;

			if (key.empty() && nPos < cursor) { // we've got a key
				key = this->_query_string.substr(nPos, cursor-nPos);
			}
			else // check for value case
				this->subProcessQueryString(lengthOfQueryString, cursor, nPos, key);

			nPos = cursor + 1;
		} else if (this->_query_string[cursor] == '&'	|| cursor+1 == lengthOfQueryString) // we have a value (potentially)
		{
			// skipping next &/=
			if (this->skipNextCharacterCheck(lengthOfQueryString, cursor, nPos, key))
				continue;

			// check for value
			this->subProcessQueryString(lengthOfQueryString, cursor, nPos, key);
			nPos = cursor+1;
		} else if (this->_query_string[cursor] == '?') // should not happen at all
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest without query string!");

		// increment cursor
		cursor++;
	}
}

inline bool tissuestack::networking::HttpRequest::skipNextCharacterCheck(int& lengthOfQueryString, int&cursor, int&nPos, std::string& key)
{
	if ((cursor+1) < lengthOfQueryString &&
			(this->_query_string[cursor+1] == '&' || this->_query_string[cursor+1] == '=')) // skip = or second &
	{
		nPos = ++cursor;
		key.clear();
		return true;
	}
	return false;
}

inline void tissuestack::networking::HttpRequest::subProcessQueryString(int& lengthOfQueryString, int&cursor, int&nPos, std::string& key)
{
	if (!key.empty() && nPos < cursor)
	{
		// delegate adding to achieve uri decode and case insensitivity
		this->addQueryParameter(
				key,
				this->_query_string.substr(
						nPos,
						((cursor+1 == lengthOfQueryString) ? lengthOfQueryString - nPos : cursor- nPos)));
		key.clear(); // reset key

		// fast forward after key/value insertion to next potential hit i.e. a '&'
		while ((cursor+1) < lengthOfQueryString && this->_query_string[cursor] != '&' && this->_query_string[cursor+1] != '&')
			nPos = ++cursor;
	}
}

inline void tissuestack::networking::HttpRequest::addQueryParameter(std::string key, std::string value)
{
	this->partiallyURIDecodeString(key);
	std::transform(key.begin(), key.end(), key.begin(), toupper);
	this->partiallyURIDecodeString(value);

	if (key.empty()) return;
	this->_parameters[key] = value;
}

inline void tissuestack::networking::HttpRequest::partiallyURIDecodeString(std::string& potentially_uri_encoded_string)
{
	// are really only after the characters that are reserved and used in tissue stack requests, no fancy, not ascii,
	// crazy foreign stuff like Asian symbols and the Tschoermaenn scharfes ss
	std::ostringstream in;
	int cursor = 0;
	int length = potentially_uri_encoded_string.length();
	while (cursor < length)
	{
		if (potentially_uri_encoded_string[cursor] == '%' && cursor+2 < length) // encountered potential uri encodeing
		{
			// convert and fast forward
			in << tissuestack::networking::HttpRequest::MinimalURIDecodingTable[potentially_uri_encoded_string.substr(cursor, 3)];
			cursor += 3;
			continue;
		}

		in << potentially_uri_encoded_string[cursor];
		cursor++;
	}
	potentially_uri_encoded_string.replace(0, length, in.str());
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
		in << "KEY [" << s.first << "]" << " => |" << s.second << "|" << std::endl;
	return in.str();
}

const std::string tissuestack::networking::HttpRequest::getContent() const
{
	return this->_query_string;
}

std::unordered_map<std::string, std::string> tissuestack::networking::HttpRequest::getParameterMap() const
{
	return this->_parameters;
}

const bool tissuestack::networking::HttpRequest::isObsolete() const
{
	// at this level we are false by default
	return false;
}
