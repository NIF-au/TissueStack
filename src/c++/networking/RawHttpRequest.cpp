#include "networking.h"

tissuestack::networking::RawHttpRequest::~RawHttpRequest()
{
	// not doing anything at the moment
};

tissuestack::networking::RawHttpRequest::RawHttpRequest(const std::string raw_content) : _content(raw_content)
{
	this->setType(tissuestack::common::Request::Type::RAW_HTTP);
};

const std::string tissuestack::networking::RawHttpRequest::getContent() const
{
	return this->_content;
};

const bool tissuestack::networking::RawHttpRequest::isObsolete() const
{
	// at this level we are false by default
	return false;
}
