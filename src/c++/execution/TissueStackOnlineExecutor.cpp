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
}

tissuestack::execution::TissueStackOnlineExecutor::TissueStackOnlineExecutor()
	: _filters(
			new tissuestack::common::RequestFilter*[3]
	        {
	        	new tissuestack::networking::HttpRequestSanityFilter(),
	           	new tissuestack::networking::TissueStackRequestFilter(),
	           	nullptr
	        }), _imageExtractor(new tissuestack::imaging::ImageExtraction<tissuestack::imaging::SimpleCacheHeuristics>) {}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::instance()
{
	if (tissuestack::execution::TissueStackOnlineExecutor::_instance == nullptr)
		tissuestack::execution::TissueStackOnlineExecutor::_instance =
				new tissuestack::execution::TissueStackOnlineExecutor();

	return tissuestack::execution::TissueStackOnlineExecutor::_instance;
}

void tissuestack::execution::TissueStackOnlineExecutor::execute(std::string request, int client_descriptor)
{
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

	}  catch (tissuestack::common::TissueStackObsoleteRequestException& obsoleteRequest)
	{
		// TODO: handle superseded requests
	}  catch (tissuestack::common::TissueStackInvalidRequestException& invalidRequest)
	{
		std::string response = "";

		if (std::strstr(invalidRequest.what(), "favicon.ico") != NULL)
			response =
					tissuestack::utils::Misc::composeHttpResponse(
							"404 Not Found", "text/plain", std::string(invalidRequest.what()));
		else
			response =
				tissuestack::utils::Misc::composeHttpResponse(
						"200 OK", "text/plain", std::string(invalidRequest.what()));
		ssize_t bytes = send(client_descriptor, response.c_str(), response.length(), 0);
		if (bytes < 0)
			tissuestack::logging::TissueStackLogger::instance()->error(
					"Error Sending 400 Bad Request: %s \n", strerror(errno));
	} catch (tissuestack::common::TissueStackException& ex)
	{
		std::string response =
				tissuestack::utils::Misc::composeHttpResponse(
						"500 Internal Server Error", "text/plain",
						std::string(ex.what()));
		ssize_t  bytes = send(client_descriptor, response.c_str(), response.length(), 0);
		if (bytes < 0)
			tissuestack::logging::TissueStackLogger::instance()->error(
								"Error Sending 500 Internal Server Error: %s \n", strerror(errno));
	}  catch (std::exception& bad)
	{
		// propagate up, they are unexpected and potentially serious!
		throw bad;
	}
}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::_instance = nullptr;

