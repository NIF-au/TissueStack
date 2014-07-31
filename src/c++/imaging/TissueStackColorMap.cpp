#include "imaging.h"

tissuestack::imaging::TissueStackColorMap::TissueStackColorMap(const std::string & filename) : _colormap_id(filename)
{
	//TODO: read color map file
}

tissuestack::imaging::TissueStackColorMap::TissueStackColorMap(const tissuestack::imaging::TissueStackLabelLookup * label_lookup_file)
{
	//TODO: convert lookup file to colormap Remember to delete !!
}

tissuestack::imaging::TissueStackColorMap::~TissueStackColorMap()
{
	//TODO: implement
}

const tissuestack::imaging::TissueStackColorMap * tissuestack::imaging::TissueStackColorMap::fromFile(const std::string & filename)
{
	return new tissuestack::imaging::TissueStackColorMap(filename);
}

const tissuestack::imaging::TissueStackColorMap * tissuestack::imaging::TissueStackColorMap::fromLabelLookup(const tissuestack::imaging::TissueStackLabelLookup * labelLookup)
{
	return new tissuestack::imaging::TissueStackColorMap(labelLookup);
}

const std::string tissuestack::imaging::TissueStackColorMap::getColorMapId() const
{
	return this->_colormap_id;
}
