#include "networking.h"

const tissuestack::common::Request& tissuestack::networking::RequestPromoter::promoteRequest(tissuestack::networking::RawHttpRequest & in) const
{
	return std::move(tissuestack::networking::HttpRequest(in));
};

const tissuestack::common::Request& tissuestack::networking::RequestPromoter::promoteRequest(tissuestack::networking::HttpRequest & in) const
{
	return in;
};
