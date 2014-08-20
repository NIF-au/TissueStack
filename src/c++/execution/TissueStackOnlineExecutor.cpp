#include "tissuestack.h"
#include "execution.h"
#include "networking.h"

tissuestack::execution::TissueStackOnlineExecutor::~TissueStackOnlineExecutor()
{
	if (this->_filters)
	{
		int i=0;
		while (this->_filters[i])
		{
			delete this->_filters[i];
			i++;
		}
		delete [] this->_filters;
	}

	if (this->_imageExtractor)
	{
		delete this->_imageExtractor;
		this->_imageExtractor = nullptr;
	}

	if (this->_serviesDelegator)
	{
		delete this->_serviesDelegator;
		this->_serviesDelegator = nullptr;
	}
}

tissuestack::execution::TissueStackOnlineExecutor::TissueStackOnlineExecutor()
	: _filters(
			new tissuestack::common::RequestFilter*[3]
	        {
	        	new tissuestack::networking::HttpRequestSanityFilter(),
	           	new tissuestack::networking::TissueStackRequestFilter(),
	           	nullptr
	        }), _imageExtractor(new tissuestack::imaging::ImageExtraction<tissuestack::imaging::SimpleCacheHeuristics>),
	        	_serviesDelegator(new tissuestack::services::TissueStackServicesDelegator()) {}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::instance()
{
	if (tissuestack::execution::TissueStackOnlineExecutor::_instance == nullptr)
		tissuestack::execution::TissueStackOnlineExecutor::_instance =
				new tissuestack::execution::TissueStackOnlineExecutor();

	return tissuestack::execution::TissueStackOnlineExecutor::_instance;
}

void tissuestack::execution::TissueStackOnlineExecutor::execute(std::string request, int client_descriptor)
{
	std::string response = "";

	try
	{
		std::unique_ptr<const tissuestack::common::Request> req(new tissuestack::networking::RawHttpRequest(request));

		int i=0;
		while (this->_filters[i])
		{
		  req.reset(this->_filters[i]->applyFilter(req.get()));
		  i++;
		}

		if (req.get()->getType() == tissuestack::common::Request::Type::TS_IMAGE)
			this->_imageExtractor->processImageRequest(
					static_cast<const tissuestack::networking::TissueStackImageRequest *>(req.get()),
					client_descriptor);
		else if (req.get()->getType() == tissuestack::common::Request::Type::TS_QUERY)
			this->_imageExtractor->processQueryRequest(
					static_cast<const tissuestack::networking::TissueStackQueryRequest *>(req.get()),
					client_descriptor);
		else if (req.get()->getType() == tissuestack::common::Request::Type::TS_SERVICES)
			this->_serviesDelegator->processRequest(
					static_cast<const tissuestack::networking::TissueStackServicesRequest *>(req.get()),
					client_descriptor);

	}  catch (tissuestack::common::TissueStackObsoleteRequestException& obsoleteRequest)
	{
		response =
				tissuestack::utils::Misc::composeHttpResponse(
					"408 Request Timeout", "text/plain", std::string(obsoleteRequest.what()));
	}  catch (tissuestack::common::TissueStackInvalidRequestException& invalidRequest)
	{
		if (std::strstr(invalidRequest.what(), "favicon.ico") != NULL)
			response =
				tissuestack::utils::Misc::composeHttpResponse(
					"404 Not Found", "text/plain", std::string(invalidRequest.what()));
		else
			response =
				tissuestack::utils::Misc::composeHttpResponse(
					"200 OK", "text/plain", std::string(invalidRequest.what()));
	} catch (tissuestack::common::TissueStackException& ex)
	{
		response =
			tissuestack::utils::Misc::composeHttpResponse(
				"500 Internal Server Error", "text/plain", std::string(ex.what()));
	}  catch (std::exception& bad)
	{
		// propagate up, they are unexpected and potentially serious!
		throw bad;
	}

	if (response.empty())
		return;

	// sending error message
	ssize_t  bytes = send(client_descriptor, response.c_str(), response.length(), 0);
	if (bytes < 0)
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Error Sending 500 Internal Server Error: %s \n", strerror(errno));
}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::_instance = nullptr;

