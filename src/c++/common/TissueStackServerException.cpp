#include "exceptions.h"

tissuestack::common::TissueStackServerException::TissueStackServerException() :
	tissuestack::common::TissueStackServerException::TissueStackServerException(std::string("Tissue Stack Server Exception")) {}

tissuestack::common::TissueStackServerException::TissueStackServerException(std::string what)
{
	this->setWhat(what);
}
