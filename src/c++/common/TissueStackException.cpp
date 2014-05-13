#include "exceptions.h"

tissuestack::common::TissueStackException::TissueStackException() : _what(nullptr) {}

tissuestack::common::TissueStackException::TissueStackException(std::string what)
{
	this->_what = new char[what.length()+1];
	std::strncpy(this->_what, what.c_str(), what.length());
	this->_what[what.length()] = '\0';
}

const char * tissuestack::common::TissueStackException::what() const throw()
{
	if (this->_what == nullptr) return static_cast<const char *>("Exception Info was not filled in");

	return static_cast<const char *>(this->_what);
}

tissuestack::common::TissueStackException::~TissueStackException() throw()
{
	delete [] this->_what;
}

