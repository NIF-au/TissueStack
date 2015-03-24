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

tissuestack::services::TissueStackService::TissueStackService() {}
tissuestack::services::TissueStackService::~TissueStackService() {}

void tissuestack::services::TissueStackService::addMandatoryParametersForRequest(
		const std::string action, const std::vector<std::string> mandatoryParams)
{
	this->_MANDATORY_PARAMETERS[action] = mandatoryParams;
}

void tissuestack::services::TissueStackService::checkMandatoryRequestParameters(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string action = request->getRequestParameter("ACTION", true);
	if (action.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"There has to be a 'action' parameter for a TissueStack Services request !");

	try
	{
		const std::vector<std::string> mandatoryParams =
				this->_MANDATORY_PARAMETERS.at(action);

		// now loop over list and check if we have all of them
		for (auto p : mandatoryParams)
			if (request->getRequestParameter(p).empty())
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
						"Mandatory parameter " + p + " is missing!");
	} catch (std::out_of_range & not_found) {
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"(A) mandatory parameter(s) for the action do(es) not exist!");
	}
}
