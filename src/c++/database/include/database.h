#ifndef	__DATABASE_H__
#define __DATABASE_H__

#include "logging.h"
#include "exceptions.h"
#include <iostream>
#include <pqxx/pqxx>

namespace tissuestack
{
	namespace database
	{
		class TissueStackPostgresConnector final
		{
			public:
				TissueStackPostgresConnector & operator=(const TissueStackPostgresConnector&) = delete;
				TissueStackPostgresConnector(const TissueStackPostgresConnector&) = delete;
				~TissueStackPostgresConnector();
				static TissueStackPostgresConnector * instance();
				const pqxx::result executeNonTransactionalQuery(const std::string sql);
		    	void purgeInstance();
		    	const bool isConnected() const;
			private:
		    	std::string _connectString;
		    	void reconnect();
		    	TissueStackPostgresConnector(
		    			const std::string host,
		    			const short port,
		    			const std::string database,
		    			const std::string user,
		    			const std::string password);
				static TissueStackPostgresConnector * _instance;
				pqxx::connection * _connection = nullptr;
	 	};

		class Configuration final
		{
			public:
				Configuration & operator=(const Configuration&) = delete;
				Configuration(const Configuration&) = delete;
				Configuration(const std::string name, const std::string value, const std::string description = "");
				const std::string getName() const;
				const std::string getValue() const;
				void setValue(const std::string value);
				const std::string getDescription() const;
				const std::string getJson() const;
			private:
				std::string _name;
				std::string _value;
				std::string _description;
		};

		class ConfigurationDataProvider final
		{
			public:
				ConfigurationDataProvider & operator=(const ConfigurationDataProvider&) = delete;
				ConfigurationDataProvider(const ConfigurationDataProvider&) = delete;
				ConfigurationDataProvider() = delete;
				static const Configuration * queryConfigurationById(const std::string name);
				static const std::vector<tissuestack::database::Configuration *> queryAllConfigurations();
			private:
				static inline tissuestack::database::Configuration * readResult(pqxx::result::const_iterator result);
		};
	}
}

#endif	/* __DATABASE_H__ */
