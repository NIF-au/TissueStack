#ifndef	__SERVICES_H__
#define __SERVICES_H__

#include "logging.h"
#include "exceptions.h"
#include "networking.h"
#include "database.h"

namespace tissuestack
{
	namespace services
	{
		class TissueStackServiceError
		{
			public:
				static const std::string NO_RESULTS;
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
