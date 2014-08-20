#include "services.h"

tissuestack::services::TissueStackServicesDelegator::TissueStackServicesDelegator()
{
	// register some standard services
	this->_registeredServices[tissuestack::services::ConfigurationService::SUB_SERVICE_ID] =
			new tissuestack::services::ConfigurationService();
}

tissuestack::services::TissueStackServicesDelegator::~TissueStackServicesDelegator()
{
	for (auto pair : this->_registeredServices)
		delete pair.second;
}

void tissuestack::services::TissueStackServicesDelegator::processRequest(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor)
{
	const tissuestack::services::TissueStackService * subService =
			this->_registeredServices[request->getSubService()];
	if (subService == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Failed to find a registered sub service to deal with this request!");

	subService->checkRequest(request);
	subService->streamResponse(request, file_descriptor);
}

