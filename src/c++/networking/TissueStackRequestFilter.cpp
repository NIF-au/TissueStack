#include "networking.h"

tissuestack::networking::TissueStackRequestFilter::~TissueStackRequestFilter()
{
	// not doing anything at the moment
};

tissuestack::networking::TissueStackRequestFilter::TissueStackRequestFilter()
{
	// not doing anything at the moment
};

const tissuestack::common::Request * const tissuestack::networking::TissueStackRequestFilter::applyFilter(const tissuestack::common::Request * const request) const
{
	if (request == nullptr)   THROW_TS_EXCEPTION(tissuestack::common::TissueStackException, "applyFilter was called with NULL");

	// we can only work with an HttpRequest object
	if (request->getType() != tissuestack::common::Request::Type::HTTP_REQUEST)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with non http request");

	const tissuestack::networking::HttpRequest * const httpRequest =
			static_cast<const tissuestack::networking::HttpRequest * const>(request);

	// we need a service parameter at a minimum
	std::string service = httpRequest->getParameter("SERVICE");
	if (service.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"A TissueStack request needs to have a \"SERVICE\" query parameter, e.g. '/?service=image' !");

	std::cout << std::endl << "SERVICE: " << service << std::endl;
	// let factory create a specific instance of TissueStackRequest based on the service string and hand in map
	// while populating its members it's checking for missing parameters and throws appropriate exceptions
	// once successful we cast the result

	// if we are an images request we will check time stamps for image requests and store our own.
	// Remark: use existing request code checking
	// if we are a conversion/pretiling we will check whether a conversion/pretiling is running already (future todo)

	// we are good => return a proper TissueStackRequest instance
	// TODO: after that we can finally start with the actual processing !!!
	return nullptr;
};
