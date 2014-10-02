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
#ifndef	__SERVICES_H__
#define __SERVICES_H__

#include "tissuestack.h"

namespace tissuestack
{
	namespace services
	{
		class TissueStackServiceError
		{
			public:
				TissueStackServiceError & operator=(const TissueStackServiceError&) = delete;
				TissueStackServiceError(const TissueStackServiceError&) = delete;
				explicit TissueStackServiceError(const std::exception & exception);
				explicit TissueStackServiceError(const tissuestack::common::TissueStackException & exception);
				const std::string toJson() const;
			private:
				const std::string _exception;
		};

		class TissueStackService
		{
			public:
				TissueStackService & operator=(const TissueStackService&) = delete;
				TissueStackService(const TissueStackService&) = delete;
				TissueStackService();
				virtual ~TissueStackService();
				virtual void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const = 0;
				virtual void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const = 0;
			protected:
				void addMandatoryParametersForRequest(const std::string action, const std::vector<std::string> mandatoryParams);
				void checkMandatoryRequestParameters(
						const tissuestack::networking::TissueStackServicesRequest * request) const;
			private:
				std::unordered_map<std::string, std::vector<std::string> > _MANDATORY_PARAMETERS;
		};

		class ConfigurationService final : public TissueStackService
		{
			public:
				static const std::string SUB_SERVICE_ID;
				ConfigurationService & operator=(const ConfigurationService&) = delete;
				ConfigurationService(const ConfigurationService&) = delete;
				ConfigurationService();
				~ConfigurationService();

				void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
	 	};

		class ColorMapService final : public TissueStackService
		{
			public:
				static const std::string SUB_SERVICE_ID;
				ColorMapService & operator=(const ColorMapService&) = delete;
				ColorMapService(const ColorMapService&) = delete;
				ColorMapService();
				~ColorMapService();

				void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
	 	};

		class DataSetConfigurationService final : public TissueStackService
		{
			public:
				static const std::string SUB_SERVICE_ID;
				DataSetConfigurationService & operator=(const DataSetConfigurationService&) = delete;
				DataSetConfigurationService(const DataSetConfigurationService&) = delete;
				DataSetConfigurationService();
				~DataSetConfigurationService();

				void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
	 	};

		enum TissueStackTaskStatus
		{
			QUEUED = 0,
			IN_PROCESS = 1,
			FINISHED = 2,
			CANCELLED = 3,
			ERRONEOUS = 4
		};

		enum TissueStackTaskType
		{
			CONVERSION = 0,
			TILING = 1
		};

		class TissueStackTask
		{
			public:
				TissueStackTask & operator=(const TissueStackTask&) = delete;
				TissueStackTask(const TissueStackTask&) = delete;
				TissueStackTask(const std::string id, const std::string input_file);

				const std::string getId() const;
				const float getProgress() const;
				const TissueStackTaskStatus getStatus() const;
				const unsigned long long int getSlicesDone() const;
				const unsigned long long int getTotalSlices() const;
				void setStatus(const TissueStackTaskStatus status);
				virtual const TissueStackTaskType getType() const = 0;
				virtual void dumpTaskToDebugLog() const = 0;
				virtual ~TissueStackTask();
				const bool isInputDataRaw() const;
				const tissuestack::imaging::TissueStackImageData * getInputImageData() const;
			protected:
				friend class TissueStackTaskQueue;
				void setSlicesDone(const unsigned long long int slicesDone);
				void setTotalSlices(const unsigned long long int totalSlices);
				friend class tissuestack::imaging::PreTiler;
				friend class tissuestack::imaging::RawConverter;
				const bool incrementSlicesDone();
			private:
				const std::string _id;
				const std::string _input_file;
				TissueStackTaskStatus _status;
				unsigned long long int _slices_done = 0;
				unsigned long long int _total_slices = 0;
				tissuestack::imaging::TissueStackImageData * _input_data = nullptr;
		};

		class TissueStackConversionTask : public TissueStackTask
		{
			public:
				TissueStackConversionTask & operator=(const TissueStackTask&) = delete;
				TissueStackConversionTask(const TissueStackTask&) = delete;
				TissueStackConversionTask(
					const std::string id,
					const std::string input_file,
					const std::string output_file = "");
				~TissueStackConversionTask();
				const std::string getOutFile() const;
				const TissueStackTaskType getType() const;
				void dumpTaskToDebugLog() const;
				const unsigned long long int getFutureRawFileSize() const;
				const unsigned long long int calculatePureDataSize() const;
			private:
				std::string _output_file;
				std::string _raw_header;
		};

		class TissueStackTilingTask : public TissueStackTask
		{
			public:
				TissueStackTilingTask & operator=(const TissueStackTilingTask&) = delete;
				TissueStackTilingTask(const TissueStackTilingTask&) = delete;
				TissueStackTilingTask(
					const std::string id,
					const std::string input_file,
					const std::string tile_dir,
					const std::vector<std::string> dimensions = {"x", "y", "z"},
					const std::vector<unsigned short> zoom_levels = {0, 1, 2, 3, 4, 5, 6, 7},
					const std::string color_map = "grey",
					const unsigned short square = 256,
					const std::string image_format = "PNG");
				~TissueStackTilingTask();
				const bool includesZoomLevel(const tissuestack::services::TissueStackTilingTask * anotherTask) const;
				const TissueStackTaskType getType() const;
				const std::string getParametersForTaskFile() const;
				void dumpTaskToDebugLog() const;
			protected:
				friend class tissuestack::imaging::PreTiler;
				const std::vector<unsigned short> getZoomLevels() const;
				const std::vector<std::string> getDimensions() const;
				const std::string getImageFormat() const;
				const std::string getColorMap() const;
				const std::string getTileDir() const;
				const unsigned int getSquareLength() const;
			private:
				std::string _tile_dir;
				std::vector<std::string> _dimensions;
				std::vector<unsigned short> _zoom_levels;
				std::string _color_map;
				unsigned short _square;
				std::string _image_format;
		};

		class TissueStackTaskQueue final
		{
			public:
				TissueStackTaskQueue & operator=(const TissueStackTaskQueue&) = delete;
				TissueStackTaskQueue(const TissueStackTaskQueue&) = delete;
				~TissueStackTaskQueue();
				static TissueStackTaskQueue * instance();
				void addTask(const TissueStackTask * task);
				void purgeInstance();
				static const bool doesInstanceExist();
				void dumpAllTasksToDebugLog() const;
				const bool doesTaskExistForDataSet(const std::string & name,
						const bool is_being_tiled_check = false, const bool is_being_converted_check = false);
				const bool isBeingTiled(const tissuestack::services::TissueStackTilingTask * check_t);
				const bool isBeingConverted(const std::string in_file);
				const TissueStackTask * findTaskById(const std::string & id);
				const TissueStackTask * getNextTask(const bool set_processing_flag = true);
				void flagTaskAsFinished(
						const std::string & task_id,
						const bool was_cancelled = false,
						const bool erase_task = true);
				void flagTaskAsErroneous(const std::string & task_id, const bool erase_task = true);
				void persistTaskProgress(const std::string & task_id);
				const std::string generateTaskId();
			private:
				TissueStackTaskQueue();
				inline void buildTaskFromIndividualTaskFile(const std::string & task_file);
				inline void writeBackToIndividualTasksFile(const tissuestack::services::TissueStackTask * task);
				inline void flagTask(
						const tissuestack::services::TissueStackTask * hit,
						const tissuestack::services::TissueStackTaskStatus status,
						const bool erase_task = true);
				inline void flagUnaddedTask(
						const tissuestack::services::TissueStackTask * hit,
						const tissuestack::services::TissueStackTaskStatus status);
				inline void eraseTask(const std::string & task_id);
				const std::vector<std::string> getTasksFromQueueFile();
				void writeTasksToQueueFile();

				std::mutex _queue_mutex;
				std::mutex _tasks_mutex;
				std::vector<const TissueStackTask *> _tasks;
				static TissueStackTaskQueue * _instance;

	 	};

		class TissueStackAdminService final : public TissueStackService
		{
			public:
				static const std::string SUB_SERVICE_ID;
				TissueStackAdminService & operator=(const TissueStackAdminService&) = delete;
				TissueStackAdminService(const TissueStackAdminService&) = delete;
				TissueStackAdminService();
				~TissueStackAdminService();

				void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
			private:
				static unsigned long long int FILE_UPLOAD_LIMIT;
				const std::string handleSetTilingRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleTaskCancellationRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleUploadRequest(
					const tissuestack::common::ProcessingStrategy * processing_strategy,
					const tissuestack::networking::TissueStackServicesRequest * request,
					int fd);
				const std::string handleDataSetAdditionRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleUploadDirectoryRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleFileExistenceRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleFileDeletionRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleFileRenameRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleDataSetRawFilesRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleUploadProgressRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const std::string handleProgressRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				const bool readAndStoreFileUploadData(
					const tissuestack::common::ProcessingStrategy * processing_strategy,
					const std::string filename,
					int socketDescriptor,
					int uploadFileDescriptor,
					unsigned int start,
					const std::string firstPartOfStream,
					const unsigned long long int supposedFileSize,
					const std::string boundary);
				int createUploadFiles(const std::string file_name, const unsigned long long int supposedFileSize);
				inline const std::string readHeaderFromRequest(
					const std::string httpMessage,
					const std::string header,
					unsigned int & endOfHeader) const;
				inline void writeUploadProgress(
						const std::string filename,
						const unsigned long long int partial,
						const unsigned long long int total
					) const;
				inline std::string readAnotherBufferFromSocketAsString(int socketDescriptor) const;
	 	};

		class TissueStackSecurityService final : public TissueStackService
		{
			public:
				static const std::string SUB_SERVICE_ID;
				static const std::string DEFAULT_GLOBAL_ADMIN_PASSWORD_AS_SHA_2_HEX_STRING;
				static const unsigned long long int DEFAULT_SESSION_TIMEOUT;
				TissueStackSecurityService & operator=(const TissueStackSecurityService&) = delete;
				TissueStackSecurityService(const TissueStackSecurityService&) = delete;
				TissueStackSecurityService();
				~TissueStackSecurityService();

				void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const;
				void streamResponse(
						const tissuestack::common::ProcessingStrategy * processing_strategy,
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
				static const bool hasSessionExpired(const std::string session);
			private:
				const bool isAdminPassword(const std::string & password) const;
				const bool setNewAdminPassword(const std::string & password) const;
				const std::string encodeSHA256(const std::string & expression) const;
	 	};

		class TissueStackServicesDelegator final
		{
			public:
				TissueStackServicesDelegator & operator=(const TissueStackServicesDelegator&) = delete;
				TissueStackServicesDelegator(const TissueStackServicesDelegator&) = delete;
				TissueStackServicesDelegator();
				~TissueStackServicesDelegator();

				void processRequest(
					const tissuestack::common::ProcessingStrategy * processing_strategy,
					const tissuestack::networking::TissueStackServicesRequest * request,
					const int file_descriptor);
			private:
				std::unordered_map<std::string, TissueStackService *> _registeredServices;
		};
	}
}

#endif	/* __SERVICES_H__ */
