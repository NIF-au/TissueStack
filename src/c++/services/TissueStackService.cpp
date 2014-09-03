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
				"The action parameter does not exist!");
	}
}
