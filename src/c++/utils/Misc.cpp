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
		if (pos == old_pos)
			pos = some_string.find(delimiter, pos+1);

		tokens.push_back(some_string.substr(old_pos, pos-old_pos));

		if (pos == std::string::npos) break;

		++pos;
		old_pos=pos;
	}

	return tokens;
}
