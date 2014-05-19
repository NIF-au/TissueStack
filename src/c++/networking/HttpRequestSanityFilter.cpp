#include "networking.h"

tissuestack::networking::HttpRequestSanityFilter::~HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

tissuestack::networking::HttpRequestSanityFilter::HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

const bool tissuestack::networking::HttpRequestSanityFilter::applyFilter(const tissuestack::common::Request & in) const
{
	return false;
};
