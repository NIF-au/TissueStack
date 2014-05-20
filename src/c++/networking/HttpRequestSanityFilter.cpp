#include "networking.h"

tissuestack::networking::HttpRequestSanityFilter::~HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

tissuestack::networking::HttpRequestSanityFilter::HttpRequestSanityFilter()
{
	// not doing anything at the moment
};

const tissuestack::common::Request * const tissuestack::networking::HttpRequestSanityFilter::applyFilter(const tissuestack::common::Request * const request) const
{
	/*
	// we test for possible good and bad cases
	try
	{
		const tissuestack::networking::RawHttpRequest& req = dynamic_cast<const tissuestack::networking::RawHttpRequest&>(in);
		// very good case: we are exactly what we need to be !
		return this->applyFilter0(req);
	}
	catch(const std::bad_cast& e)
	{
		// second try ...
		try
		{
			dynamic_cast<const tissuestack::networking::RawHttpRequest&>(in);
			// we are good too, in fact we don't need to be filtered any more
			return in;
		}
		catch(const std::bad_cast& e) // bad: we cannot cope with you
		{
			throw tissuestack::common::TissueStackInvalidRequestException("Request could not be cast to fit Filter's needs");
		}
	}*/

	return nullptr;
};

const tissuestack::common::Request * const tissuestack::networking::HttpRequestSanityFilter::applyFilter0(const tissuestack::networking::RawHttpRequest * const raw_request) const
{
	// TODO: perform tests and throw exception if failed
	return nullptr;
};
