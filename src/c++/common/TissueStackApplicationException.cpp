#include "exceptions.h"

tissuestack::common::TissueStackApplicationException::TissueStackApplicationException(std::string what) :
	tissuestack::common::TissueStackException(what) {}
