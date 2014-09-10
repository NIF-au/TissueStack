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
	if (request == nullptr)   THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException, "applyFilter was called with NULL");

	// we can only work with an HttpRequest object
	if (request->getType() != tissuestack::common::Request::Type::HTTP)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "applyFilter was called with non http request");

	const tissuestack::networking::HttpRequest * const httpRequest =
			static_cast<const tissuestack::networking::HttpRequest * const>(request);
	//httpRequest->dumpParametersIntoDebugLog();


	// we need a service parameter at a minimum
	std::string service = httpRequest->getParameter("SERVICE");
	if (service.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"A TissueStack request needs to have a \"SERVICE\" query parameter, e.g. '/?service=image' !");

	// upper case for better comparison
	std::transform(service.begin(), service.end(), service.begin(), toupper);

	std::unordered_map<std::string, std::string> parameters = httpRequest->getParameterMap();

	// instantiate the appropriate request class
	tissuestack::common::Request * return_request = nullptr;
	if (tissuestack::networking::TissueStackImageRequest::SERVICE1.compare(service) == 0)
		return_request = new tissuestack::networking::TissueStackImageRequest(parameters, false);
	else if (tissuestack::networking::TissueStackImageRequest::SERVICE2.compare(service) == 0)
		return_request = new tissuestack::networking::TissueStackImageRequest(parameters, true);
	else if (tissuestack::networking::TissueStackQueryRequest::SERVICE.compare(service) == 0)
		return_request = new tissuestack::networking::TissueStackQueryRequest(parameters);
	else if (tissuestack::networking::TissueStackServicesRequest::SERVICE.compare(service) == 0)
	{
		return_request =
			httpRequest->isFileUpload() ?
					new tissuestack::networking::TissueStackServicesRequest(parameters, httpRequest->getFileUploadStart()) :
					new tissuestack::networking::TissueStackServicesRequest(parameters);
	}
	else if (tissuestack::networking::TissueStackPreTilingRequest::SERVICE.compare(service) == 0)
			return_request = new tissuestack::networking::TissueStackPreTilingRequest(parameters);
	else if (tissuestack::networking::TissueStackConversionRequest::SERVICE.compare(service) == 0)
			return_request = new tissuestack::networking::TissueStackConversionRequest(parameters);

	if (return_request == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
						"A TissueStack request has to be: 'IMAGE', 'IMAGE_PREVIEW, 'QUERY', 'TILING','CONVERSION' or 'SERVICES'!");

	// a general isObsolete check. for most but not all requests that equates to a superseded timestamp check
	// for conversion/tiling, this can be used to catch duplicate conversion/tiling requests
	if (return_request->isObsolete())
	{
		delete return_request;
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
			"The TissueStack Request has become obsolete!");
	}

	return return_request;
};
