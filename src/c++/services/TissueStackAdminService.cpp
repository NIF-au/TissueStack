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

const std::string tissuestack::services::TissueStackAdminService::SUB_SERVICE_ID = "ADMIN";
unsigned long long int tissuestack::services::TissueStackAdminService::FILE_UPLOAD_LIMIT = 10000000000; // ~ 10 gig by default
tissuestack::services::TissueStackAdminService::TissueStackAdminService() {
	/* do not need session */
	this->addMandatoryParametersForRequest("UPLOAD_DIRECTORY", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("DATA_SET_RAW_FILES", std::vector<std::string>{});
	this->addMandatoryParametersForRequest("UPLOAD_PROGRESS",
		std::vector<std::string>{ "FILE"});
	this->addMandatoryParametersForRequest("PROGRESS",
		std::vector<std::string>{ "TASK_ID"});
	/* need session */
	this->addMandatoryParametersForRequest("UPLOAD",
		std::vector<std::string>{ "SESSION"});
	this->addMandatoryParametersForRequest("ADD_DATASET",
		std::vector<std::string>{ "SESSION" , "FILENAME"});
	this->addMandatoryParametersForRequest("TOGGLE_TILING",
		std::vector<std::string>{ "SESSION" , "ID", "FLAG"});
	this->addMandatoryParametersForRequest("CANCEL",
		std::vector<std::string>{ "SESSION" , "TASK_ID"});
};

tissuestack::services::TissueStackAdminService::~TissueStackAdminService() {};

void tissuestack::services::TissueStackAdminService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::TissueStackAdminService::streamResponse(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::string json = tissuestack::common::NO_RESULTS_JSON;

	// do the requests that need no session first
	if (action.compare("UPLOAD_DIRECTORY") == 0)
		json = this->handleUploadDirectoryRequest(request);
	else if (action.compare("DATA_SET_RAW_FILES") == 0)
		json = this->handleDataSetRawFilesRequest(request);
	else if (action.compare("UPLOAD_PROGRESS") == 0)
		json = this->handleUploadProgressRequest(request);
	else if (action.compare("PROGRESS") == 0)
		json = this->handleProgressRequest(request);
	else
	{
		// the following resources need a valid session
		if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
			request->getRequestParameter("SESSION")))
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Invalid Session! Please Log In.");
		}

		if (action.compare("TOGGLE_TILING") == 0)
			json = this->handleSetTilingRequest(request);
		else if (action.compare("CANCEL") == 0)
			json = this->handleTaskCancellationRequest(request);
		else if (action.compare("UPLOAD") == 0)
			json =
				const_cast<tissuestack::services::TissueStackAdminService *>(this)->handleUploadRequest(
					processing_strategy,
					request,
					file_descriptor);
		else if (action.compare("ADD_DATASET") == 0)
			json = this->handleDataSetAdditionRequest(request);
	}

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json);
	write(file_descriptor, response.c_str(), response.length());
}

const std::string tissuestack::services::TissueStackAdminService::handleSetTilingRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const unsigned long long int id =
		strtoull(request->getRequestParameter("ID", true).c_str(), NULL, 10);

	const std::string flag =
		request->getRequestParameter("FLAG", true);

	const bool response =
		tissuestack::database::DataSetDataProvider::setIsTiledFlag(
			id, (flag.compare("TRUE") == 0) ? true : false);

	if (response)
		return "{\"response\":\"Toggled pre-tiling/image flag successfully\"}";
	else
		return tissuestack::common::NO_RESULTS_JSON;
}

const std::string tissuestack::services::TissueStackAdminService::handleTaskCancellationRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string task_id =
		request->getRequestParameter("TASK_ID", true);

	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(task_id);
	if (hit == nullptr)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");

	const std::string fileName =
		hit->getInputImageData()->getFileName();
	const std::string progress = std::to_string(hit->getProgress());
	// cancel
	tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(task_id, true, false);

	return std::string("{\"response\": {\"filename\": \"") +
		fileName + "\", \"progress\":" +
		progress + "}}";
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadRequest(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::networking::TissueStackServicesRequest * request,
	int socketDescriptor)
{
	if (!tissuestack::utils::System::directoryExists(UPLOAD_PATH) &&
			!tissuestack::utils::System::createDirectory(UPLOAD_PATH, 0755))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not create upload directory!");

	// extract content type & content length & content disposition
	unsigned int streamPointer = 0;
	std::string httpStreamFrame = request->getFileUploadStart();

	const std::string contentType =
		this->readHeaderFromRequest(httpStreamFrame, "Content-Type:", streamPointer);
	unsigned int pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
				"File Upload is not multipart with a defined boundary!");
	const std::string boundary = std::string("\r\n\r\n--") + contentType.substr(pos+9) + "--";

	std::string contentLength =
		this->readHeaderFromRequest(httpStreamFrame, "Content-Length:", streamPointer);
	if (contentLength.empty() &&
			httpStreamFrame.length() < tissuestack::common::SOCKET_READ_BUFFER_SIZE)
	{
		// it seems we have to read more ...
		httpStreamFrame = this->readAnotherBufferFromSocketAsString(socketDescriptor);
		contentLength =
			this->readHeaderFromRequest(httpStreamFrame, "Content-Length:", streamPointer);
	}

	if (contentLength.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"Could not find header 'Content-Length' of file upload!");

	unsigned long long int contentLengthInBytes =
		strtoull(tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(
			contentLength.substr(15)).c_str(), NULL, 10);
	if (contentLengthInBytes == 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"Content-Length of file upload is 0!");

	std::string contentDisposition =
		this->readHeaderFromRequest(httpStreamFrame, "Content-Disposition:", streamPointer);
	if (contentDisposition.empty() &&
			httpStreamFrame.length() < tissuestack::common::SOCKET_READ_BUFFER_SIZE)
	{
		// it seems we have to read more ...
		httpStreamFrame = this->readAnotherBufferFromSocketAsString(socketDescriptor);
		contentDisposition =
				this->readHeaderFromRequest(httpStreamFrame, "Content-Disposition:", streamPointer).substr(15);
	}
	if (contentDisposition.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"Could not find header 'Content-Disposition' of file upload!");

	pos = contentDisposition.find("filename=");
	if (pos == std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"File Name for file upload is missing!");
	pos += 10;
	contentDisposition = contentDisposition.substr(pos);
	pos = contentDisposition.find("\"");
	if (pos == std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"File Name for file upload is missing!");
	const std::string fileName = contentDisposition.substr(0, pos);

	if (tissuestack::utils::System::fileExists(std::string(UPLOAD_PATH) + "/" + fileName))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
		"File already exists in the upload folder!");

	//tissuestack::logging::TissueStackLogger::instance()->debug("ALL: |%s|", httpStreamFrame.c_str());
	//tissuestack::logging::TissueStackLogger::instance()->debug("******************************");
	//tissuestack::logging::TissueStackLogger::instance()->debug("BOUNDARY: |%s|", boundary.c_str());
	//tissuestack::logging::TissueStackLogger::instance()->debug("CONTENT-LENGTH: |%llu|", contentLengthInBytes);
	//tissuestack::logging::TissueStackLogger::instance()->debug("FILENAME: |%s|", fileName.c_str());

	// create a temporary upload file to store progress
	int uploadFileDescriptor = this->createUploadFiles(fileName, contentLengthInBytes);

	std::unique_ptr<const tissuestack::database::Configuration> max_upload_size(
		tissuestack::database::ConfigurationDataProvider::queryConfigurationById("max_upload_size"));
	if (max_upload_size)
		tissuestack::services::TissueStackAdminService::FILE_UPLOAD_LIMIT =
			strtoull(max_upload_size->getValue().c_str(), NULL, 10);

		if (!this->readAndStoreFileUploadData(
			processing_strategy,
			fileName,
			socketDescriptor,
			uploadFileDescriptor,
			streamPointer,
			httpStreamFrame,
			contentLengthInBytes,
			boundary))
		{
			// we have been cancelled, delete the incomplete file
			unlink((std::string(UPLOAD_PATH) + "/" + fileName).c_str());
			unlink((std::string(UPLOAD_PATH) + "/." + fileName + ".upload").c_str());
			return "{ \"response\": \"Upload of file '" + fileName + "' cancelled!\"}";
		};

		return "{ \"response\": \"Upload of file '" + fileName + "' finished\"}";
}

int tissuestack::services::TissueStackAdminService::createUploadFiles(
	const std::string file_name, const unsigned long long int supposedFileSize)
{
	int fd =
		open((std::string(UPLOAD_PATH) + "/." + file_name + ".upload" ).c_str(),
		O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <= 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not write temporary upload progress file!");
	const std::string progress = std::string("0/") + std::to_string(supposedFileSize) + "\n";
	if (write(fd, progress.c_str(), progress.size()) < 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not write content of upload progress file!");
	close(fd);

	fd =
		open((std::string(UPLOAD_PATH) + "/" + file_name).c_str(),
		O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <= 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not write upload file!");
	return fd;
}

const bool tissuestack::services::TissueStackAdminService::readAndStoreFileUploadData(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const std::string filename,
	int socketDescriptor,
	int uploadFileDescriptor,
	unsigned int start,
	const std::string firstPartOfStream,
	const unsigned long long int supposedFileSize,
	const std::string boundary)
{
	std::string tmp = firstPartOfStream;
	ssize_t bytesReceived = 1;
	unsigned long long int totalBytesOfFileUpload = 0;

	if (processing_strategy->isStopFlagRaised())
		return false;

	// read till start of actual message
	while (bytesReceived > 0)
	{
		if (tmp.size() < tissuestack::common::SOCKET_READ_BUFFER_SIZE)
			bytesReceived = 0;

		unsigned int actualStart =
				tmp.find("\r\n\r\n", start);
		if (actualStart != std::string::npos)
		{
			tmp =  tmp.substr(actualStart+4);
			write(uploadFileDescriptor, tmp.c_str(), tmp.size());
			totalBytesOfFileUpload += static_cast<unsigned long long int>(tmp.size());
			break;
		}

		if (tmp.size() < tissuestack::common::SOCKET_READ_BUFFER_SIZE)
			break;

		// read more
		tmp = readAnotherBufferFromSocketAsString(socketDescriptor);
		if (tmp.empty())
			bytesReceived = 0;
	}

	unsigned long long int lastWriteAtBytes = totalBytesOfFileUpload;

	while (bytesReceived > 0)
	{
		if (processing_strategy->isStopFlagRaised())
			return false;

		char buffer[tissuestack::common::SOCKET_READ_BUFFER_SIZE];
		bytesReceived = recv(socketDescriptor, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0)
		{
			write(uploadFileDescriptor, buffer, bytesReceived);
			totalBytesOfFileUpload += static_cast<unsigned long long int>(bytesReceived);

			if (bytesReceived < tissuestack::common::SOCKET_READ_BUFFER_SIZE &&
					((totalBytesOfFileUpload + tissuestack::common::SOCKET_READ_BUFFER_SIZE) > supposedFileSize))
				break;
		}

		// write progress update every MB
		if ((totalBytesOfFileUpload-lastWriteAtBytes) > static_cast<unsigned long long int>(1024* 1000))
		{
			this->writeUploadProgress(filename, totalBytesOfFileUpload, supposedFileSize);
			lastWriteAtBytes = totalBytesOfFileUpload;
		}

		if (totalBytesOfFileUpload > tissuestack::services::TissueStackAdminService::FILE_UPLOAD_LIMIT)
			return false;
	}

	if (totalBytesOfFileUpload > static_cast<unsigned long long int>(boundary.size()))
	{
		unsigned long long int actualSize =
			totalBytesOfFileUpload-boundary.length();
		if (ftruncate(uploadFileDescriptor, actualSize) != 0)
			return false;
		this->writeUploadProgress(filename, actualSize, actualSize);
	}
	close(uploadFileDescriptor);

	return true;
}

inline std::string tissuestack::services::TissueStackAdminService::readAnotherBufferFromSocketAsString(
	int socketDescriptor) const
{
	char buffer[tissuestack::common::SOCKET_READ_BUFFER_SIZE];
	ssize_t bytesReceived = recv(socketDescriptor, buffer, sizeof(buffer), 0);
	if (bytesReceived > 0)
		return std::string(buffer, bytesReceived);

	return "";
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetAdditionRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string fileName = request->getRequestParameter("FILENAME");
	const std::string description = request->getRequestParameter("DESCRIPTION");

	const size_t pos = fileName.find_last_of("/");
	std::string finalDestination = DATASET_PATH;
	std::string absoluteFileName = fileName;
	if (pos != std::string::npos)
		finalDestination += ("/" + fileName.substr(pos+1));
	else
	{
		finalDestination += ("/" + fileName);
		absoluteFileName = std::string(UPLOAD_PATH) + "/" + fileName;
	}

	// check for existing file in data directory
	if (!tissuestack::utils::System::fileExists(absoluteFileName))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Data Set file does not exist!");

	if (tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(finalDestination))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Data Set referring to same file exists already!");

	// move raw file !
	int status = rename(absoluteFileName.c_str(), finalDestination.c_str());
	if (status != 0)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to mv file: %s", strerror(errno));
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not move data set: This could be due to lack of permissions or upload and data directory residing in different partitions!");
	}

	std::unique_ptr<const tissuestack::imaging::TissueStackImageData> dataSet;
	unsigned long long int id = 0;
	try
	{
		dataSet.reset(tissuestack::imaging::TissueStackImageData::fromFile(finalDestination));
		if (!dataSet->isRaw())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "Only .raw files can be added!");

		if (tissuestack::database::DataSetDataProvider::addDataSet(
				dataSet.get(), description) != (dataSet->getNumberOfDimensions() + 1))
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failed to persist all dimensions of the data set!");

		// now that we are persisted => add us to the memory data store
		id = dataSet->getDataBaseId();
		tissuestack::imaging::TissueStackDataSetStore::instance()->addDataSet(
			tissuestack::imaging::TissueStackDataSet::fromTissueStackImageData(dataSet.release()));

		/* this is not needed, strictly speaking s unless we have a reason, we'll omit it!
		// now go into the db to double check and retrieve default values
		const tissuestack::imaging::TissueStackImageData * correspondingRecord =
				tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(dataSet->getDataBaseId(), true);

		// integrate the record
		std::vector<const tissuestack::imaging::TissueStackImageData *> tmp;
		tmp.push_back(correspondingRecord);
		tissuestack::imaging::TissueStackDataSetStore::instance()->integrateDataBaseResultsIntoDataSetStore(tmp);
		*/
	} catch (const std::exception & e)
	{
		// rollback
		try
		{
			if (id != 0)
			{
				if (tissuestack::database::DataSetDataProvider::eraseDataSet(id))
					tissuestack::imaging::TissueStackDataSetStore::instance()->removeDataSetByDataBaseId(id);
			}
		} catch(...)
		{
			// ignored
		}
		// undo moving of file
		rename(finalDestination.c_str(), absoluteFileName.c_str());

		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to persist data set: %s", e.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failed to persist data set!");
	}

	std::string json = "{\"response\": ";
	json +=
		tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(finalDestination)->getImageData()->toJson(true)
		+ "}";

	return json;
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadDirectoryRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string sDisplayRawOnly =
			request->getRequestParameter("DISPLAY_RAW_ONLY", true);
	const bool bDisplayRawOnly =
		(!sDisplayRawOnly.empty() && sDisplayRawOnly.compare("TRUE") == 0) ?
			true : false;
	const std::string sDisplayConversionFormatsOnly =
			request->getRequestParameter("DISPLAY_CONVERSION_FORMATS_ONLY", true);
	const bool bDisplayConversionFormatsOnly =
		(!sDisplayConversionFormatsOnly.empty() && sDisplayConversionFormatsOnly.compare("TRUE") == 0) ?
			true : false;

	std::vector<std::string> results;

	const std::vector<std::string> filesInUploadDirectory =
		tissuestack::utils::System::getFilesInDirectory(UPLOAD_PATH);

	for (auto f : filesInUploadDirectory)
	{
		if (tissuestack::services::TissueStackTaskQueue::instance()->doesTaskExistForDataSet(f, false, true))
			continue;

		if (bDisplayRawOnly)
		{
			if (f.length() < 4)
				continue;

			if (!(f.substr(f.length()-4).compare(".raw") == 0
					|| f.substr(f.length()-4).compare(".RAW") == 0))
				continue;
		}

		if (bDisplayConversionFormatsOnly)
		{
			if (f.length() < 4)
				continue;
			std::string ext = f.substr(f.length()-4);
			std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

			if (!(ext.compare(".MNC") == 0
					|| ext.compare(".NII") == 0
					|| ext.compare("I.GZ") == 0))
				continue;
		}

		const unsigned int pos = f.find_last_of("/");
		const std::string tmp =
			(pos != std::string::npos) ?
				f.substr(pos+1) :
				f;

		if (tmp.at(0) == '.') // no hidden files
			continue;

		// if we have a file in upload progress => don't show it
		if (tissuestack::utils::System::fileExists(std::string(UPLOAD_PATH) + "/." + tmp + ".upload"))
			continue;

		results.push_back(tmp);
	}

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto r : results)
	{
		if (i !=0)
			json << ",";
		json << "\"" << r << "\"";
		i++;
	}

	json << "]}";

	return json.str();
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetRawFilesRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	std::vector<std::string> results;

	const std::vector<std::string> dataSets =
		tissuestack::imaging::TissueStackDataSetStore::instance()->getRawFileDataSetFiles();

	for (auto ds : dataSets)
	{
		if (tissuestack::services::TissueStackTaskQueue::instance()->doesTaskExistForDataSet(ds, true, false))
			continue;

		const unsigned int pos = ds.find_last_of("/");
		if (pos != std::string::npos)
			results.push_back(ds.substr(pos+1));
		else
			results.push_back(ds);
	}

	std::ostringstream json;
	json << "{\"response\": [";

	unsigned int i=0;
	for (auto r : results)
	{
		if (i !=0)
			json << ",";
		json << "\"" << r << "\"";
		i++;
	}

	json << "]}";

	return json.str();
}

const std::string tissuestack::services::TissueStackAdminService::handleUploadProgressRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	if (!tissuestack::utils::System::directoryExists(UPLOAD_PATH) &&
			!tissuestack::utils::System::createDirectory(UPLOAD_PATH, 0755))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not create upload directory!");

	const std::string filename = request->getRequestParameter("FILE");

	int fd = -1;
	{
		fd =
			open((std::string(UPLOAD_PATH) + "/." + filename + ".upload" ).c_str(),
			O_RDONLY, 0644);
		if (fd <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
				"Could not open temporary upload progress file!");
	}

	char buffer[tissuestack::common::SOCKET_READ_BUFFER_SIZE];
	ssize_t bytesRead = read(fd, buffer, tissuestack::common::SOCKET_READ_BUFFER_SIZE);
	if (bytesRead <= 0)
		return std::string("{\"response\": {\"filename\": \"") +
			filename + "\", \"progress\": -1}}";

	const std::string progress =
		tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(
			std::string(buffer, bytesRead));
	close(fd);


	const std::vector<std::string> tokens =
		tissuestack::utils::Misc::tokenizeString(progress, '/');
	if (tokens.size() != 2)
		return std::string("{\"response\": {\"filename\": \"") +
			filename + "\", \"progress\": -1}}";

	float fProgress = 0;
	// we are finished
	if (tokens[0].compare(tokens[1]) == 0)
	{
		unlink((std::string(UPLOAD_PATH) + "/." + filename + ".upload").c_str());
		fProgress = 100;
	} else
		fProgress =
			static_cast<float>(atof(tokens[0].c_str()) / atof(tokens[1].c_str())) *
				static_cast<float>(100);

	return std::string("{\"response\": {\"filename\": \"") +
			filename + "\", \"progress\":" +
			std::to_string(fProgress) + "}}";
}

const std::string tissuestack::services::TissueStackAdminService::handleProgressRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string task_id =
		request->getRequestParameter("TASK_ID", true);

	const tissuestack::services::TissueStackTask * hit =
		tissuestack::services::TissueStackTaskQueue::instance()->findTaskById(task_id);
	if (hit == nullptr)
	{
		// check if we happen to be finished, cancelled or erroneous
		const std::string pathToFile = std::string(TASKS_PATH) + "/" + task_id;

		if (tissuestack::utils::System::fileExists(pathToFile + ".done"))
			return
				std::string("{\"response\": {\"filename\": \"") +
					task_id + "\", \"progress\": 100, \"status\": " +
					std::to_string(tissuestack::services::TissueStackTaskStatus::FINISHED) +
					"}}";

		if (tissuestack::utils::System::fileExists(pathToFile + ".cancelled"))
			return
				std::string("{\"response\": {\"filename\": \"") +
					task_id + "\", \"status\": " +
					std::to_string(tissuestack::services::TissueStackTaskStatus::CANCELLED) +
					"}}";

		if (tissuestack::utils::System::fileExists(pathToFile + ".error"))
			return
				std::string("{\"response\": {\"filename\": \"") +
					task_id + "\", \"status\": " +
					std::to_string(tissuestack::services::TissueStackTaskStatus::ERRONEOUS) +
					"}}";

		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Task does not exist!");
	}

	return std::string("{\"response\": {\"filename\": \"") +
		hit->getInputImageData()->getFileName() + "\", \"progress\":" +
		std::to_string(hit->getProgress()) + ", \"status\": " +
		std::to_string(hit->getStatus()) + "}}";
}

inline void tissuestack::services::TissueStackAdminService::writeUploadProgress(
		const std::string filename,
		const unsigned long long int partial,
		const unsigned long long int total
	) const
{
	{
		int fd =
			open((std::string(UPLOAD_PATH) + "/." + filename + ".upload" ).c_str(),
			O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd <= 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not write temporary upload progress file!");
		const std::string progress =
			std::to_string(partial) + "/" + std::to_string(total) + "\n";
		if (write(fd, progress.c_str(), progress.size()) < 0)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not write content of upload progress file!");
		close(fd);
	}
}


inline const std::string tissuestack::services::TissueStackAdminService::readHeaderFromRequest(
	const std::string httpMessage,
	const std::string header,
	unsigned int & endOfHeader) const
{
	unsigned long int pos = httpMessage.find(header);
	if (pos == std::string::npos)
		return "";

	unsigned int posEnd = httpMessage.find("\r\n", pos);
	if (posEnd == std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
			"Could not find end of line for http header!");

	endOfHeader = posEnd;

	return httpMessage.substr(pos, posEnd-pos);
}
