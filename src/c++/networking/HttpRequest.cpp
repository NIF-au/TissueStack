/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "networking.h"

std::unordered_map<std::string, std::string> tissuestack::networking::HttpRequest::MinimalURIDecodingTable
{
		{"%20", " "}, // blank
		{"%26", "&"}, // &
		{"%27", "'"}, // '
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

	const std::string raw_content = raw_request->getContent();

	// check if we have an upload
	if (raw_content.find("POST") == 0 &&
			raw_content.find("service=services") != std::string::npos &&
			raw_content.find("sub_service=admin") != std::string::npos &&
			raw_content.find("action=upload") != std::string::npos)
	{
		this->_isFileUpload = true;
		this->_fileUploadStart = raw_content;
	}

	unsigned int start = this->isFileUpload() ? 5 : 4;
	// we go on to dissect the GET/POST request, we really don't care for any other http method
	size_t nPos = raw_content.find(' ', start);
	// if we are under 3/4, there is something wrong, the URI needs to start at position 3/4 => 'GET/POST /somequerystring'
	if (nPos < start)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest with malformed GET/POST");

	// now cut out query string
	this->_query_string = raw_content.substr(start,nPos-start);

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
	int lengthOfQueryString = this->_query_string.length();
	int cursor = 0, nPos = 0;
	std::string key = "";

	while (cursor < lengthOfQueryString)
	{
		if (this->_query_string[cursor] == '=') // we have a key or value (potentially)
		{
			// skipping next &/=
			int cursorBefore = this->skipNextCharacterCheck(lengthOfQueryString, cursor, nPos, key);

			if (key.empty() && nPos < cursor)  // we've got a key
				key = this->_query_string.substr(nPos, cursor-nPos);
			else
				this->subProcessQueryString(lengthOfQueryString, cursorBefore, nPos, key);

			nPos = cursor + 1;
		} else if (this->_query_string[cursor] == '&' || cursor+1 == lengthOfQueryString) // we have a value (potentially)
		{
			// skipping next &/=
			int cursorBefore = this->skipNextCharacterCheck(lengthOfQueryString, cursor, nPos, key);
			if (cursorBefore == cursor && cursor+1 == lengthOfQueryString && this->_query_string[cursor] != '&' )
				cursorBefore++;
			this->subProcessQueryString(lengthOfQueryString, cursorBefore, nPos, key);
			nPos = cursor+1;
		} else if (this->_query_string[cursor] == '?') // should not happen at all
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "HttpRequest without query string!");

		// increment cursor
		cursor++;
	}
}

inline int tissuestack::networking::HttpRequest::skipNextCharacterCheck(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key)
{
	int oldCursor = cursor;

	while ((cursor+1) < lengthOfQueryString &&
			(this->_query_string[cursor+1] == '&' ||
					this->_query_string[cursor+1] == '=')) // skip = or second &
		cursor++;

	return oldCursor;
}

inline void tissuestack::networking::HttpRequest::subProcessQueryString(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key)
{
	if (!key.empty() && nPos <= cursor)
	{
		// delegate adding to achieve uri decode and case insensitivity
		this->addQueryParameter(
				key,
				this->_query_string.substr(
						nPos,
						cursor- nPos));
		key.clear(); // reset key

		// fast forward after key/value insertion to next potential hit i.e. a '&'
		while ((cursor+1) < lengthOfQueryString && this->_query_string[cursor] != '&' && this->_query_string[cursor+1] != '&')
			nPos = ++cursor;
	}
}

inline void tissuestack::networking::HttpRequest::addQueryParameter(std::string & key, std::string value)
{
	this->partiallyURIDecodeString(key);
	this->partiallyURIDecodeString(value);

	if (key.empty()) return;
	// make key upper case for easier search
	std::transform(key.begin(), key.end(), key.begin(), toupper);
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

const std::string tissuestack::networking::HttpRequest::getParameter(std::string name, const bool convertToUpperCase) const
{
	try
	{
		// upper case for better comparison
		std::transform(name.begin(), name.end(), name.begin(), toupper);

		if (!convertToUpperCase)
			return this->_parameters.at(name);

		std::string value = this->_parameters.at(name);
		std::transform(value.begin(), value.end(), value.begin(), toupper);

		return value;
	} catch (const std::out_of_range& ignored) { }
	return std::string("");
}

void tissuestack::networking::HttpRequest::dumpParametersIntoDebugLog() const
{
	std::ostringstream in;
	for (auto s : this->_parameters)
		in << "KEY [" << s.first.c_str() << "]" << " => |" << s.second.c_str() << "|" << std::endl;

	const std::string out = in.str();

	tissuestack::logging::TissueStackLogger::instance()->debug("Parameters:\n%s", out.c_str());
}

const std::string tissuestack::networking::HttpRequest::getContent() const
{
	return this->_query_string;
}

const std::string tissuestack::networking::HttpRequest::getFileUploadStart() const
{
	return this->_fileUploadStart;
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

const bool tissuestack::networking::HttpRequest::isFileUpload() const
{
	return this->_isFileUpload;
}
