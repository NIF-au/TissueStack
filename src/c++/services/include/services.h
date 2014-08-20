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
		class TissueStackService
		{
			public:
				TissueStackService & operator=(const TissueStackService&) = delete;
				TissueStackService(const TissueStackService&) = delete;
				TissueStackService() {};
				virtual ~TissueStackService() {};
				virtual void checkRequest(const tissuestack::networking::TissueStackServicesRequest * request) const = 0;
				virtual void streamResponse(
						const tissuestack::networking::TissueStackServicesRequest * request,
						const int file_descriptor) const = 0;
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
