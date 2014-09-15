#include "exceptions.h"

tissuestack::common::TissueStackFileUploadException::TissueStackFileUploadException(std::string what) :
	tissuestack::common::TissueStackApplicationException(what) {}
