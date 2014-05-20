#include "networking.h"

tissuestack::networking::RawHttpRequest::~RawHttpRequest()
{
	// not doing anything at the moment
};

tissuestack::networking::RawHttpRequest::RawHttpRequest(const std::string * const raw_content)
{

};

const std::string tissuestack::networking::RawHttpRequest::getContent() const
{
	return this->_content;
};
