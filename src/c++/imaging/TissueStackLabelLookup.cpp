#include "imaging.h"

tissuestack::imaging::TissueStackLabelLookup::TissueStackLabelLookup(const std::string & filename) : _labellookup_id(filename)
{
	//TODO: read lookup file
}

tissuestack::imaging::TissueStackLabelLookup::~TissueStackLabelLookup()
{
	//TODO: implement
}

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::imaging::TissueStackLabelLookup::fromFile(const std::string & filename)
{
	return new tissuestack::imaging::TissueStackLabelLookup(filename);
}

const std::string tissuestack::imaging::TissueStackLabelLookup::getLabelLookupId() const
{
	return this->_labellookup_id;
}
