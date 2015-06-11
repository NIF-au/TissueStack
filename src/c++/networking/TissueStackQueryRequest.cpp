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

const std::string tissuestack::networking::TissueStackQueryRequest::SERVICE = "QUERY";

tissuestack::networking::TissueStackQueryRequest::TissueStackQueryRequest(
		std::unordered_map<std::string, std::string> & request_parameters)
{
	this->setTimeStampInfoFromRequestParameters(request_parameters);
	this->setDataSetFromRequestParameters(request_parameters);
	this->setDimensionFromRequestParameters(request_parameters);
	this->setSliceFromRequestParameters(request_parameters);
	this->setCoordinatesFromRequestParameters(request_parameters, true);

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_QUERY);
}

const std::string tissuestack::networking::TissueStackQueryRequest::getContent() const
{
	return std::string("TS_QUERY");
}
