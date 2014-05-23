#include "tissuestack.h"

tissuestack::common::Request::Request() : _type(tissuestack::common::Request::Type::RAW_HTTP) {};

tissuestack::common::Request::~Request()
{
	// nothing to be done, after all we are abstract
};

const tissuestack::common::Request::Type tissuestack::common::Request::getType() const
{
	return this->_type;
}

void tissuestack::common::Request::setType(tissuestack::common::Request::Type type)
{
	this->_type = type;
}
