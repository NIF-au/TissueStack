#include "exceptions.h"

tissuestack::common::TissueStackNullPointerException::TissueStackNullPointerException() :
	tissuestack::common::TissueStackNullPointerException::TissueStackNullPointerException(std::string("Something is NULL")) {}

tissuestack::common::TissueStackNullPointerException::TissueStackNullPointerException(std::string what)
{
	this->setWhat(what);
}
