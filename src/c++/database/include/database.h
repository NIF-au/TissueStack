#ifndef	__DATABASE_H__
#define __DATABASE_H__

#include "logging.h"
#include "exceptions.h"
#include <iostream>
#include <pqxx/pqxx>

namespace tissuestack
{
	namespace imaging
	{
		class TissueStackImageData;
	}

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
				static const std::vector<const tissuestack::database::Configuration *> queryAllConfigurations();
			private:
				static inline tissuestack::database::Configuration * readResult(pqxx::result::const_iterator result);
		};

		class DataSetDataProvider final
		{
			public:
				DataSetDataProvider & operator=(const DataSetDataProvider&) = delete;
				DataSetDataProvider(const DataSetDataProvider&) = delete;
				DataSetDataProvider() = delete;
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> queryAll(
						const bool includePlanes = false,
						const unsigned short offset = 0,
						const unsigned short max_records = MAX_RECORDS);
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> queryById(
						const unsigned long long int id,
						const bool includePlanes = false);
			private:
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> findResults(
						const std::string sql);
				static void findAndAddPlanes(
						const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData);
				static const std::string SQL;
				static const std::string SQL_PLANES;
				static const std::string ORDER_BY;
				static const unsigned short MAX_RECORDS;
		};
	}
}

#endif	/* __DATABASE_H__ */
