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
	this->addMandatoryParametersForRequest("FILE_EXISTS",
		std::vector<std::string>{ "SESSION", "FILE"});
	this->addMandatoryParametersForRequest("FILE_DELETE",
		std::vector<std::string>{ "SESSION", "FILE"});
	this->addMandatoryParametersForRequest("FILE_RENAME",
		std::vector<std::string>{ "SESSION", "FILE", "NEW_FILE"});
	this->addMandatoryParametersForRequest("DELETE_DATASET",
		std::vector<std::string>{ "SESSION", "ID"});
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
		else if (action.compare("DELETE_DATASET") == 0)
				json = this->handleDataSetDeletionRequest(request);
		else if (action.compare("FILE_EXISTS") == 0)
			json = this->handleFileExistenceRequest(request);
		else if (action.compare("FILE_DELETE") == 0)
			json = this->handleFileDeletionRequest(request);
		else if (action.compare("FILE_RENAME") == 0)
			json = this->handleFileRenameRequest(request);
	}

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json);
	write(file_descriptor, response.c_str(), response.length());
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetDeletionRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const unsigned long long int id =
		strtoull(request->getRequestParameter("ID", true).c_str(), NULL, 10);

	const tissuestack::imaging::TissueStackDataSet * dataSet =
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSetByDataBaseId(id);
	if (dataSet == nullptr)
		return std::string("{\"response\": {\"result\": \"given dataset (id) does not exist!\"}}");
	const std::string file = dataSet->getDataSetId();

	const std::string sDeleteFile =
			request->getRequestParameter("DELETE_FILE", true);
	const bool bDeleteFile =
		(!sDeleteFile.empty() && sDeleteFile.compare("TRUE") == 0) ?
			true : false;

	tissuestack::imaging::TissueStackDataSetStore::instance()->removeDataSetByDataBaseId(id);
	tissuestack::database::DataSetDataProvider::eraseDataSet(id);
	if (bDeleteFile)
		unlink(file.c_str());

	return std::string("{\"response\": {\"result\": \"dataset deleted\"}}");
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
		hit->getInputImageData() ?
			hit->getInputImageData()->getFileName() :
			hit->getInputFileName();
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
	const std::string dir = tissuestack::services::TissueStackAdminService::getUploadDirectory();
	if (!tissuestack::utils::System::directoryExists(dir) &&
			!tissuestack::utils::System::createDirectory(dir, 0755))
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
		httpStreamFrame = this->readAnotherBufferFromSocketAsString(processing_strategy, socketDescriptor);
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
		httpStreamFrame = this->readAnotherBufferFromSocketAsString(processing_strategy, socketDescriptor);
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

	// for better comparison
	std::string lowerCaseFileName = fileName;
	std::transform(lowerCaseFileName.begin(), lowerCaseFileName.end(), lowerCaseFileName.begin(), tolower);
	if (fileName.rfind(".mnc") == std::string::npos && fileName.rfind(".nii") == std::string::npos &&
			fileName.rfind(".nii.gz") == std::string::npos && fileName.rfind(".raw") == std::string::npos &&
			fileName.rfind(".dcm") == std::string::npos && fileName.rfind(".ima") == std::string::npos &&
			fileName.rfind(".zip") == std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackFileUploadException,
		"Uploaded file needs to be of the following type: .mnc, .nii, .nii.gz, .ima, .dcm, .zip or .raw!");

	if (tissuestack::utils::System::fileExists(std::string(dir) + "/" + fileName))
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
	const unsigned int boundaryLength = boundary.length();

	if (!this->readAndStoreFileUploadData(
		processing_strategy,
		fileName,
		socketDescriptor,
		uploadFileDescriptor,
		streamPointer,
		httpStreamFrame,
		contentLengthInBytes - boundaryLength,
		boundaryLength))
	{
		// we have been cancelled, delete the incomplete file
		unlink((dir + "/" + fileName).c_str());
		unlink((dir + "/." + fileName + ".upload").c_str());
		return "{ \"response\": \"Upload of file '" + fileName + "' cancelled!\"}";
	};

	return "{ \"response\": \"Upload of file '" + fileName + "' finished\"}";
}

int tissuestack::services::TissueStackAdminService::createUploadFiles(
	const std::string file_name, const unsigned long long int supposedFileSize)
{
	const std::string dir = tissuestack::services::TissueStackAdminService::getUploadDirectory();
	int fd =
		open((dir + "/." + file_name + ".upload" ).c_str(),
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
		open((dir + "/" + file_name).c_str(),
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
	const unsigned int boundaryLength)
{
	std::string tmp = firstPartOfStream;
	unsigned long long int totalBytesOfFileUpload = 0;

	// read till start of actual content
	while (true)
	{
		unsigned int actualStart =
				tmp.find("\r\n\r\n", start);
		if (actualStart != std::string::npos)
		{
			tmp =  tmp.substr(actualStart+4);
			write(uploadFileDescriptor, tmp.c_str(), tmp.size());
			totalBytesOfFileUpload += static_cast<unsigned long long int>(tmp.size());
			break;
		}

		// read more so that we get to beginning of content
		start = 0;
		tmp = readAnotherBufferFromSocketAsString(processing_strategy, socketDescriptor);
		if (tmp.empty())
			return false;
	}

	unsigned long long int lastWriteAtBytes = totalBytesOfFileUpload;

	short eagainFailures = 0;
	while (true)
	{
		if (totalBytesOfFileUpload > tissuestack::services::TissueStackAdminService::FILE_UPLOAD_LIMIT)
			return false;
	
		errno = 0;
		tmp = readAnotherBufferFromSocketAsString(processing_strategy, socketDescriptor);
		if (tmp.empty() && errno == EAGAIN)
		{
			if (eagainFailures == 2)
				break;
			eagainFailures++;
			sleep(1);
			continue;
		}
		eagainFailures = 0;

		write(uploadFileDescriptor, tmp.c_str(), tmp.size());
		totalBytesOfFileUpload += static_cast<unsigned long long int>(tmp.size());

		if ((totalBytesOfFileUpload-lastWriteAtBytes) > static_cast<unsigned long long int>(1024* 1000))
		{
			this->writeUploadProgress(filename, totalBytesOfFileUpload, supposedFileSize);
			lastWriteAtBytes = totalBytesOfFileUpload;
		}
	}

	bool ret = false;
	const unsigned long long int delta = llabs(supposedFileSize-totalBytesOfFileUpload);
	if (delta < 250)
	{
		unsigned long long int actualSize =
			totalBytesOfFileUpload - boundaryLength;
		if (ftruncate(uploadFileDescriptor, actualSize) != 0)
			return false;
		this->writeUploadProgress(filename, actualSize, actualSize);
		ret = true;
	}
	close(uploadFileDescriptor);

	return ret;
}

inline std::string tissuestack::services::TissueStackAdminService::readAnotherBufferFromSocketAsString(
	const tissuestack::common::ProcessingStrategy * processing_strategy, int socketDescriptor) const
{
	char buffer[tissuestack::common::SOCKET_READ_BUFFER_SIZE];

	while (!processing_strategy->isStopFlagRaised())
	{
		ssize_t bytesReceived = recv(socketDescriptor, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0)
			return std::string(buffer, bytesReceived);
		break;
	}

	return "";
}

const std::string tissuestack::services::TissueStackAdminService::handleDataSetAdditionRequest(
	const tissuestack::networking::TissueStackServicesRequest * request) const
{
	const std::string fileName = request->getRequestParameter("FILENAME");
	const std::string description = request->getRequestParameter("DESCRIPTION");

	const size_t pos = fileName.find_last_of("/");
	std::string finalDestination = tissuestack::imaging::TissueStackDataSetStore::getDataSetStoreDirectory();
	std::string absoluteFileName = fileName;
	if (pos != std::string::npos)
		finalDestination += ("/" + fileName.substr(pos+1));
	else
	{
		finalDestination += ("/" + fileName);
		absoluteFileName = tissuestack::services::TissueStackAdminService::getUploadDirectory() + "/" + fileName;
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

const std::string tissuestack::services::TissueStackAdminService::handleFileExistenceRequest(const tissuestack::networking::TissueStackServicesRequest * request) const
{
	std::string file = request->getRequestParameter("FILE");

	// we only allow this to happen in the upload and data directory
	if ((file.find(tissuestack::services::TissueStackAdminService::getUploadDirectory()) == std::string::npos &&
			file.find(tissuestack::imaging::TissueStackDataSetStore::getDataSetStoreDirectory()) == std::string::npos) ||
			file.find("..") != std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Only files in the data and upload directory are allowed to be queried!");

	if (tissuestack::utils::System::fileExists(file))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"File exists already!");

	return tissuestack::common::NO_RESULTS_JSON;
}

const std::string tissuestack::services::TissueStackAdminService::handleFileDeletionRequest(const tissuestack::networking::TissueStackServicesRequest * request) const
{
	std::string file = request->getRequestParameter("FILE");

	// we only allow this to happen in the upload and data directory
	if ((file.find(tissuestack::services::TissueStackAdminService::getUploadDirectory()) == std::string::npos &&
			file.find(tissuestack::imaging::TissueStackDataSetStore::getDataSetStoreDirectory()) == std::string::npos) ||
			file.find("..") != std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Only files in the data and upload directory are allowed to be deleted!");

	if (!tissuestack::utils::System::fileExists(file))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"File does not exist!");

	if (unlink(file.c_str()) < 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not unlink file!");

	return tissuestack::common::NO_RESULTS_JSON;
}

const std::string tissuestack::services::TissueStackAdminService::handleFileRenameRequest(const tissuestack::networking::TissueStackServicesRequest * request) const
{
	std::string file = request->getRequestParameter("FILE");
	std::string new_file = request->getRequestParameter("NEW_FILE");

	// we only allow this to happen in the upload directory
	const std::string dir = tissuestack::services::TissueStackAdminService::getUploadDirectory();
	if (file.find(dir) == std::string::npos ||
			new_file.find(dir) == std::string::npos ||
			file.find("..") != std::string::npos)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Only files in the upload directory are allowed to be renamed!");

	if (!tissuestack::utils::System::fileExists(file))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"File does not exist!");

	new_file = tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(new_file);
	if (new_file.length() < 5 ||
			new_file.substr(new_file.length()-4).compare(".raw") != 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"New file name needs to be non-empty with extension .raw");

	if (rename(file.c_str(), new_file.c_str()) < 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not rename file!");

	return tissuestack::common::NO_RESULTS_JSON;
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

	const std::string dir = tissuestack::services::TissueStackAdminService::getUploadDirectory();
	const std::vector<std::string> filesInUploadDirectory =
		tissuestack::utils::System::getFilesInDirectory(dir);

	for (auto f : filesInUploadDirectory)
	{
		if (tissuestack::services::TissueStackTaskQueue::instance()->doesTaskExistForDataSet(f, false, true))
			continue;

		if (f.length() < 4)
			continue;

		std::string ext = f.substr(f.length()-4);
		std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

		if (bDisplayRawOnly &&
				(!(ext.compare(".RAW") == 0)))
			continue;

		if (bDisplayConversionFormatsOnly &&
			(!(ext.compare(".MNC") == 0
				|| ext.compare(".NII") == 0
				|| ext.compare("I.GZ") == 0
				|| ext.compare(".DCM") == 0
				|| ext.compare(".IMA") == 0
				|| ext.compare(".ZIP") == 0)))
			continue;

		// don't show files that are at the moment being converted
		if (ext.compare(".RAW") == 0 && tissuestack::services::TissueStackTaskQueue::instance()->isBeingConverted(f))
			continue;

		const unsigned int pos = f.find_last_of("/");
		const std::string tmp =
			(pos != std::string::npos) ?
				f.substr(pos+1) :
				f;

		if (tmp.at(0) == '.') // no hidden files
			continue;

		// if we have a file in upload progress => don't show it
		if (tissuestack::utils::System::fileExists(std::string(dir) + "/." + tmp + ".upload"))
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
	const std::string dir = tissuestack::services::TissueStackAdminService::getUploadDirectory();
	if (!tissuestack::utils::System::directoryExists(dir) &&
			!tissuestack::utils::System::createDirectory(dir, 0755))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not create upload directory!");

	const std::string filename = request->getRequestParameter("FILE");

	int fd = -1;
	{
		fd =
			open((std::string(dir) + "/." + filename + ".upload" ).c_str(),
			O_RDONLY);
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
		unlink((std::string(dir) + "/." + filename + ".upload").c_str());
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
		const std::string pathToFile = tissuestack::services::TissueStackTaskQueue::getTasksDirectory() + "/" + task_id;

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
		(hit->getInputImageData() == nullptr ?
			hit->getInputFileName() :
			hit->getInputImageData()->getFileName()) +
			"\", \"progress\":" +
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
			open((tissuestack::services::TissueStackAdminService::getUploadDirectory() + "/." + filename + ".upload" ).c_str(),
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

const std::string tissuestack::services::TissueStackAdminService::getUploadDirectory()
{
	std::string dir =
		tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory("upload_directory");
	if (dir.empty())
		dir = UPLOAD_PATH;

	return dir;
}
