#include "exceptions.h"

tissuestack::common::TissueStackNullPointerException::TissueStackNullPointerException(std::string what) :
	tissuestack::common::TissueStackApplicationException(what) {}
