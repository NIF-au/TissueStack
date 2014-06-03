#include "exceptions.h"

tissuestack::common::TissueStackObsoleteRequestException::TissueStackObsoleteRequestException(std::string what) :
	tissuestack::common::TissueStackInvalidRequestException(what) {}
