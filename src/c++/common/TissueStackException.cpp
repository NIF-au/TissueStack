#include "exceptions.h"

tissuestack::common::TissueStackException::TissueStackException() : _what("Exception not properly specified!") {}

/*
void tissuestack::common::TissueStackException::setWhat(const std::string what)
{
	this->_what = new char[what.length()+1];
	std::strncpy(this->_what, what.c_str(), what.length());
	this->_what[what.length()] = '\0';
}*/
tissuestack::common::TissueStackException::TissueStackException(const std::string what) : _what(what) {}

const char * tissuestack::common::TissueStackException::what() const throw()
{
	if (this->_what.empty()) return static_cast<const char *>("Generic Tissue Stack Application Exception");

	return this->_what.c_str();
}
/*
tissuestack::common::TissueStackException::~TissueStackException() throw()
{
	if (this->_what)
	{
		delete [] this->_what;
		this->_what = nullptr;
	}
}*/

