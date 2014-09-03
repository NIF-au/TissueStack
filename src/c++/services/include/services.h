#ifndef	__SERVICES_H__
#define __SERVICES_H__

#include "logging.h"
#include "exceptions.h"
#include "networking.h"
#include "database.h"
#include "imaging.h"
#include <memory>

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
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
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
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const;
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
					const tissuestack::networking::TissueStackServicesRequest * request,
					const int file_descriptor);
			private:
				std::unordered_map<std::string, TissueStackService *> _registeredServices;
		};
	}
}

#endif	/* __SERVICES_H__ */
