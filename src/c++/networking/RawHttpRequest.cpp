#include "networking.h"

tissuestack::networking::RawHttpdRequest::RawHttpdRequest() {};
tissuestack::networking::RawHttpdRequest::~RawHttpdRequest() {};

tissuestack::networking::RawHttpdRequest::RawHttpdRequest(std::string raw_content)
{
	//throw tissuestack::common::TissueStackInvalidRequestException(std::string("bad request"));
};

const std::string tissuestack::networking::RawHttpdRequest::getRawContent() const
{
	return std::string("NULL");
};
