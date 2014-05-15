#include "exceptions.h"

tissuestack::common::TissueStackInvalidRequestException::TissueStackInvalidRequestException() :
	tissuestack::common::TissueStackInvalidRequestException::TissueStackInvalidRequestException(std::string("Invalid Request")) {}

tissuestack::common::TissueStackInvalidRequestException::TissueStackInvalidRequestException(std::string what)
{
	this->setWhat(what);
}
