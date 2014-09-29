/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
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
	if (request == nullptr)   THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException, "applyFilter was called with NULL");

	if (request->getType() != tissuestack::common::Request::Type::RAW_HTTP)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with non raw request");

	// quick check if we begin the right way ...
	const std::string raw_content = request->getContent();

	// there need to be 14 characters at a bare minimum, e.g. GET / HTTP/X.X
	// of course, there better be more but we check that later
	if (raw_content.length() < 10)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with incomplete http request");

	// apart from a file upload we are not interested in any NON GET type of requests
	if (!(raw_content.find("POST") == 0 &&
		raw_content.find("service=services") != std::string::npos &&
		raw_content.find("sub_service=admin") != std::string::npos &&
		raw_content.find("action=upload") != std::string::npos) &&
			raw_content.find("GET") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Tissue Stack only wants to deal with GET requests");

	// annoying favicon
	if (raw_content.find("favicon.ico") != std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Tissue Stack does not like favicon.ico requests");


	// we are good => return a proper HttpRequest instance
	return new tissuestack::networking::HttpRequest(static_cast<const tissuestack::networking::RawHttpRequest * const>(request), true);;
};
