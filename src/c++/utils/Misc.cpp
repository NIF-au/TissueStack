#include "utils.h"

tissuestack::utils::Misc::Misc() {}

const std::string tissuestack::utils::Misc::convertCharPointerToString(char * some_characters)
{
	if (some_characters == nullptr) return std::string("");

	std::ostringstream in;
	in << some_characters;

	return in.str();
}

const std::string tissuestack::utils::Misc::demangleTypeIdName(const char * mangeledIdName)
{
	if (mangeledIdName == nullptr) return std::string("");

	int status = 0;
	char * buffer = NULL;
	unsigned long int length = strlen(mangeledIdName);

	char * demangled_name = abi::__cxa_demangle(mangeledIdName, buffer, &length, &status);
	if (status != 0) return std::string("");

	// copy over result
	std::string ret(tissuestack::utils::Misc::convertCharPointerToString(demangled_name));
	free(demangled_name);

	return ret;
}

const std::vector<std::string> tissuestack::utils::Misc::tokenizeString(const std::string & some_string, const char delimiter)
{
	std::vector<std::string> tokens;

	if (some_string.empty())
		return tokens;

	size_t old_pos = 0;
	size_t pos = 0;

	while (1)
	{
		pos = some_string.find(delimiter, pos);

		// peek ahead to avoid repeated appearances of the delimiter
		if (pos+1 < some_string.length() && some_string.at(pos+1) == delimiter)
		{
			++pos;
			++old_pos;
			continue;
		}

		if (pos == old_pos)
			pos = some_string.find(delimiter, pos+1);

		tokens.push_back(some_string.substr(old_pos, pos-old_pos));

		if (pos == std::string::npos) break;

		++pos;
		old_pos=pos;
	}

	return tokens;
}

const std::string tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(
		std::unordered_map<std::string, std::string> & map, std::string key)
{
	std::transform(key.begin(), key.end(), key.begin(), toupper);

	try
	{
		return map.at(key);
	} catch (const std::out_of_range& ignored) { }

	return std::string("");
}

const std::string tissuestack::utils::Misc::composeHttpResponse(
		std::string status, std::string content_type, std::string content)
{
	const std::string CR_LF = "\r\n";
	std::ostringstream response;

	response << "HTTP/1.1 " << status << CR_LF; // HTTTP/1.1 status
	response << "Date: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // Date (in the past)
	response << "Connection: close" << CR_LF; // Connection header (close)
	response << "Server: Tissue Stack Image Server" <<  CR_LF; // Server header
	response << "Accept-Ranges: bytes" << CR_LF; // Accept-Ranges header
	response << "Content-Type: " << content_type << CR_LF; // Content-Type header
	response << "Last-Modified: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // last modified header in the past
	response << "Access-Control-Allow-Origin: *" << CR_LF; // allow cross origin requests

	if (!content.empty())
	{
		response << "Content-Length: " << content.length() << CR_LF << CR_LF; // Content-Length header
		response << content;
	} else response <<  CR_LF;

	return response.str();
}
