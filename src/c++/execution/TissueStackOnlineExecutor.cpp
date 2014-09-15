#include "execution.h"
#include "networking.h"
#include "imaging.h"
#include "services.h"

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

	if (this->_tissueStackRawConverter)
	{
		delete this->_tissueStackRawConverter;
		this->_tissueStackRawConverter = nullptr;
	}

	if (this->_tissueStackPreTiler)
	{
		delete this->_tissueStackPreTiler;
		this->_tissueStackPreTiler = nullptr;
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
	        	_serviesDelegator(new tissuestack::services::TissueStackServicesDelegator()),
	        	_tissueStackRawConverter(new tissuestack::imaging::RawConverter()),
	        	_tissueStackPreTiler(new tissuestack::imaging::PreTiler()) {}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::instance()
{
	if (tissuestack::execution::TissueStackOnlineExecutor::_instance == nullptr)
		tissuestack::execution::TissueStackOnlineExecutor::_instance =
				new tissuestack::execution::TissueStackOnlineExecutor();

	return tissuestack::execution::TissueStackOnlineExecutor::_instance;
}

void tissuestack::execution::TissueStackOnlineExecutor::execute(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const std::string request,
		int client_descriptor)
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

		if (req.get()->getType() == tissuestack::common::Request::Type::TS_IMAGE) /* IMAGE REQUEST */
			this->_imageExtractor->processImageRequest(
					processing_strategy,
					static_cast<const tissuestack::networking::TissueStackImageRequest *>(req.get()),
					client_descriptor);
		else if (req.get()->getType() == tissuestack::common::Request::Type::TS_QUERY) /* QUERY REQUEST */
			this->_imageExtractor->processQueryRequest(
					static_cast<const tissuestack::networking::TissueStackQueryRequest *>(req.get()),
					client_descriptor);
		else if (req.get()->getType() == tissuestack::common::Request::Type::TS_SERVICES) /* SERVICES REQUEST */
			this->_serviesDelegator->processRequest(
					processing_strategy,
					static_cast<const tissuestack::networking::TissueStackServicesRequest *>(req.get()),
					client_descriptor);
		else if (req.get()->getType() == tissuestack::common::Request::Type::TS_CONVERSION ||
				req.get()->getType() == tissuestack::common::Request::Type::TS_TILING)
		{
			/* CONVERSION/PRE-TILING REQUEST */
			const tissuestack::services::TissueStackTask * task =
				(req.get()->getType() == tissuestack::common::Request::Type::TS_CONVERSION) ?
					static_cast<const tissuestack::services::TissueStackTask *>(
						(const_cast<tissuestack::networking::TissueStackConversionRequest *>(
							(static_cast<
								const tissuestack::networking::TissueStackConversionRequest *>(req.get()))))->getTask(true)) :
					static_cast<const tissuestack::services::TissueStackTask *>(
						(const_cast<tissuestack::networking::TissueStackPreTilingRequest *>(
							(static_cast<
								const tissuestack::networking::TissueStackPreTilingRequest *>(req.get()))))->getTask(true));
			tissuestack::services::TissueStackTaskQueue::instance()->addTask(task);

			response =
				std::string("{\"response\": \"") + task->getId() + + "\"}";
			response =
				tissuestack::utils::Misc::composeHttpResponse(
					"200 OK",
					"application/json",
					response);
		}
	}  catch (tissuestack::common::TissueStackObsoleteRequestException& obsoleteRequest) /* ERRONEOUS REQUESTS */
	{
		response =
				tissuestack::utils::Misc::composeHttpResponse(
					"408 Request Timeout",
					"application/json",
					tissuestack::services::TissueStackServiceError(obsoleteRequest).toJson());
	}  catch (tissuestack::common::TissueStackInvalidRequestException& invalidRequest)
	{
		if (std::strstr(invalidRequest.what(), "favicon.ico") != NULL)
			response =
				tissuestack::utils::Misc::composeHttpResponse(
					"404 Not Found",
					"application/json",
					tissuestack::services::TissueStackServiceError(invalidRequest).toJson());
		else
		{
			tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute Process: %s\n", invalidRequest.what());
			response =
				tissuestack::utils::Misc::composeHttpResponse(
					"200 OK",
					"application/json",
					tissuestack::services::TissueStackServiceError(invalidRequest).toJson());
			shutdown(client_descriptor, SHUT_RD);
		}
	}  catch (tissuestack::common::TissueStackFileUploadException& uploadException)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to upload a file: %s\n", uploadException.what());
		response =
			tissuestack::utils::Misc::composeHttpResponse(
				"413 Request Entity Too Large",
				"application/json",
				tissuestack::services::TissueStackServiceError(uploadException).toJson());
	} catch (tissuestack::common::TissueStackApplicationException& ex)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute Process: %s\n", ex.what());
		response =
			tissuestack::utils::Misc::composeHttpResponse(
				"200 OK",
				"application/json",
				tissuestack::services::TissueStackServiceError(ex).toJson());
	} catch (tissuestack::common::TissueStackException& ex)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute Process: %s\n", ex.what());
		response =
			tissuestack::utils::Misc::composeHttpResponse(
				"500 Internal Server Error",
				"application/json",
				tissuestack::services::TissueStackServiceError(ex).toJson());
	}  catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute Process: %s\n", bad.what());
		response =
			tissuestack::utils::Misc::composeHttpResponse(
				"500 Internal Server Error",
				"application/json",
				tissuestack::services::TissueStackServiceError(bad).toJson());
	}

	if (response.empty())
	{
		close(client_descriptor);
		return;
	}

	// sending error message
	ssize_t  bytes = send(client_descriptor, response.c_str(), response.length(), 0);
	if (bytes < 0)
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Error Sending 500 Internal Server Error: %s \n", strerror(errno));
	close(client_descriptor);
}

void tissuestack::execution::TissueStackOnlineExecutor::executeTask(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackTask * task)
{
	if (task == nullptr || processing_strategy == nullptr)
		return;

	if (task->getType() == tissuestack::services::TissueStackTaskType::CONVERSION)
		this->_tissueStackRawConverter->convert(
			processing_strategy,
			static_cast<const tissuestack::services::TissueStackConversionTask *>(task));
	else if (task->getType() == tissuestack::services::TissueStackTaskType::TILING)
		this->_tissueStackPreTiler->preTile(
			processing_strategy,
			static_cast<const tissuestack::services::TissueStackTilingTask *>(task));
}

tissuestack::execution::TissueStackOnlineExecutor * tissuestack::execution::TissueStackOnlineExecutor::_instance = nullptr;

