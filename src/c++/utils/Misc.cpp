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
