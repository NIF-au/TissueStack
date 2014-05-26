#include "exceptions.h"

tissuestack::common::TissueStackObsoleteRequestException::TissueStackObsoleteRequestException() :
	tissuestack::common::TissueStackObsoleteRequestException::TissueStackObsoleteRequestException(std::string("Request Expired")) {}

tissuestack::common::TissueStackObsoleteRequestException::TissueStackObsoleteRequestException(std::string what)
{
	this->setWhat(what);
}
