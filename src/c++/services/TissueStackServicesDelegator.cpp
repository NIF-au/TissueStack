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

tissuestack::services::TissueStackServicesDelegator::TissueStackServicesDelegator()
{
	// register some standard services
	this->_registeredServices[tissuestack::services::TissueStackSecurityService::SUB_SERVICE_ID] =
			new tissuestack::services::TissueStackSecurityService();
	this->_registeredServices[tissuestack::services::TissueStackAdminService::SUB_SERVICE_ID] =
			new tissuestack::services::TissueStackAdminService();
	this->_registeredServices[tissuestack::services::ConfigurationService::SUB_SERVICE_ID] =
			new tissuestack::services::ConfigurationService();
	this->_registeredServices[tissuestack::services::ColorMapService::SUB_SERVICE_ID] =
			new tissuestack::services::ColorMapService();
	this->_registeredServices[tissuestack::services::DataSetConfigurationService::SUB_SERVICE_ID] =
			new tissuestack::services::DataSetConfigurationService();
	this->_registeredServices[tissuestack::services::TissueStackMetaDataService::SUB_SERVICE_ID] =
			new tissuestack::services::TissueStackMetaDataService();
}

tissuestack::services::TissueStackServicesDelegator::~TissueStackServicesDelegator()
{
	for (auto pair : this->_registeredServices)
		delete pair.second;
}

void tissuestack::services::TissueStackServicesDelegator::processRequest(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor)
{
	const tissuestack::services::TissueStackService * subService =
			this->_registeredServices[request->getSubService()];
	if (subService == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Failed to find a registered sub service to deal with this request!");

	subService->checkRequest(request);
	subService->streamResponse(processing_strategy, request, file_descriptor);
}

