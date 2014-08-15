#include "networking.h"

const std::string tissuestack::networking::TissueStackQueryRequest::SERVICE = "QUERY";

tissuestack::networking::TissueStackQueryRequest::TissueStackQueryRequest(
		std::unordered_map<std::string, std::string> & request_parameters)
{
	this->setTimeStampInfoFromRequestParameters(request_parameters);
	this->setDataSetFromRequestParameters(request_parameters);
	this->setDimensionFromRequestParameters(request_parameters);
	this->setSliceFromRequestParameters(request_parameters);
	this->setCoordinatesFromRequestParameters(request_parameters);

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_QUERY);
}

const std::string tissuestack::networking::TissueStackQueryRequest::getContent() const
{
	return std::string("TS_QUERY");
}
