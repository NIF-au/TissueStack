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
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackServiceError::TissueStackServiceError(
		const std::exception & exception) : _exception(std::string(exception.what())) {}

tissuestack::services::TissueStackServiceError::TissueStackServiceError(
		const tissuestack::common::TissueStackException & exception) : _exception(std::string(exception.what())) {}

const std::string tissuestack::services::TissueStackServiceError::toJson() const
{
	std::ostringstream json;

	json << "{\"error\": { \"exception\": \"";
	std::string sWhat(this->_exception);
	std::string sException = "Exception";
	std::string sDescription = sWhat;

	if (sWhat.empty()) // not specified
	{
		sException = "N/A";
		sDescription = "An unexpected exception without further description occurred";
	} else if (sWhat.at(sWhat.length()-1) == ']')
	{
		// if it is a tissuestack exception we can dissect it into a description and exception part
		size_t iPos = sWhat.rfind("@ LINE:");
		if (iPos != std::string::npos &&
			((iPos = sWhat.rfind("[", iPos)) != std::string::npos))
		{
			sDescription = sWhat.substr(0, iPos-1);
			sException = sWhat.substr(iPos, sWhat.length()-iPos);
		}
	}
	if (sDescription.find("ERROR: ") == 0) // remove leading ERROR: term
		sDescription = sDescription.substr(7);
	json << tissuestack::utils::Misc::maskQuotesInJson(sException) << "\", \"description\": \"";
	json << tissuestack::utils::Misc::maskQuotesInJson(sDescription) << "\"}}";

	return json.str();
}
