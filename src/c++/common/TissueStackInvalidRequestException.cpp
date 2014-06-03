#include "exceptions.h"

tissuestack::common::TissueStackInvalidRequestException::TissueStackInvalidRequestException(std::string what) :
	tissuestack::common::TissueStackApplicationException(what) {}
