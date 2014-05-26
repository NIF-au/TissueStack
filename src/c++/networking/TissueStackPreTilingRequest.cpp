#include "networking.h"

const std::string tissuestack::networking::TissueStackPreTilingRequest::SERVICE = "TILE";


tissuestack::networking::TissueStackPreTilingRequest::~TissueStackPreTilingRequest() {}

tissuestack::networking::TissueStackPreTilingRequest::TissueStackPreTilingRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	// TODO: we check and assign to the according members
	THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "NOT IMPLEMENTED YET!");

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_TILING);

}

const bool tissuestack::networking::TissueStackPreTilingRequest::isObsolete() const
{
	return false;
}

const std::string tissuestack::networking::TissueStackPreTilingRequest::getContent() const
{
	return std::string("TS_TILING");
}
