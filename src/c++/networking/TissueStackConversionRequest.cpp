#include "networking.h"

const std::string tissuestack::networking::TissueStackConversionRequest::SERVICE = "CONVERSION";


tissuestack::networking::TissueStackConversionRequest::~TissueStackConversionRequest() {}

tissuestack::networking::TissueStackConversionRequest::TissueStackConversionRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	// TODO: we check and assign to the according members
	THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "NOT IMPLEMENTED YET!");

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_CONVERSION);

}

const bool tissuestack::networking::TissueStackConversionRequest::isObsolete() const
{
	// TODO: this should indicate that the conversion has already taken place
	// implement that
	return false;
}

const std::string tissuestack::networking::TissueStackConversionRequest::getContent() const
{
	return std::string("TS_CONVERSION");
}
