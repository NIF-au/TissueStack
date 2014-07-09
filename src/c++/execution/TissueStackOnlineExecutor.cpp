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

}

tissuestack::execution::TissueStackOnlineExecutor::TissueStackOnlineExecutor()
	: _filters(
			new tissuestack::common::RequestFilter*[3]
	        {
	        	new tissuestack::networking::HttpRequestSanityFilter(),
	           	new tissuestack::networking::TissueStackRequestFilter(),
	           	nullptr
	        }) {}

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
	}  catch (tissuestack::common::TissueStackObsoleteRequestException& obsoleteRequest)
	{
		// TODO: handle superseded requests
		std::cerr << obsoleteRequest.what() << std::endl;
	}  catch (tissuestack::common::TissueStackInvalidRequestException& invalidRequest)
	{
		// TODO: return reason for invalid requests
		std::string response = this->composeHttpResponse("400 Bad Request", "text/plain", std::string(invalidRequest.what()));
		ssize_t bytes = send(client_descriptor, response.c_str(), response.length(), 0);
		if (bytes < 0)
			perror("Send error:");
	}  catch (tissuestack::common::TissueStackException& ex)
	{
		std::string response = this->composeHttpResponse("500 Internal Server Error", "text/plain", std::string(ex.what()));
		ssize_t  bytes = send(client_descriptor, response.c_str(), response.length(), 0);
		if (bytes < 0)
			perror("Send error");

		// TODO: log
		std::cerr << ex.what() << std::endl;
	}  catch (std::exception& bad)
	{
		// propagate up, they are unexpected and potentially serious!
		throw bad;
	}
}

std::string tissuestack::execution::TissueStackOnlineExecutor::composeHttpResponse(
		std::string status, std::string content_type, std::string content)
{
	const std::string CR_LF = "\r\n";
	std::ostringstream response;

	response << "HTTP/1.1 " << status << CR_LF; // HTTTP/1.1 status
	response << "Server: Tissue Stack Image Server" <<  CR_LF; // Server header
	response << "Connection: close" << CR_LF; // Connection header (close)
	response << "Date: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // Date (in the past)
	response << "Last-Modified: Thu, 20 May 2004 21:12:11 GMT" << CR_LF; // last modified header in the past
	response << "Access-Control-Allow-Origin: *" << CR_LF; // allow cross origin requests
	response << "Accept-Ranges: bytes" << CR_LF; // Accept-Ranges header
	response << "Content-Type: " << content_type << CR_LF; // Content-Type header

	if (!content.empty())
	{
		response << "Content-Length: " << content.length() << CR_LF << CR_LF; // Content-Length header
		response << content;
	}

	return response.str();
}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::_instance = nullptr;

