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
#ifndef	__DATABASE_H__
#define __DATABASE_H__

#include "tissuestack.h"
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
				static const bool doesInstanceExist();
				const pqxx::result executeNonTransactionalQuery(const std::string sql);
				const unsigned long long int executeTransaction(const std::vector<std::string> sql);
				const pqxx::result executePaginatedQuery(
					const std::string sql,
					const unsigned int from,
					const unsigned int to);
		    	void purgeInstance();
		    	const bool isTransConnected() const;
		    	const bool isNonTransBackupConnected() const;
		    	const bool isNonTransConnected(const unsigned short index);
			private:
		    	static const unsigned short MAX_NON_TRANSACTIONAL_CONNECTIONS = 5;
		    	std::atomic_bool _busyNonTransactionalConnections[MAX_NON_TRANSACTIONAL_CONNECTIONS];
		    	std::mutex _transactionMutex;
		    	std::mutex _non_transactionMutex;
		    	std::string _connectString;
		    	void disconnectTransConnection();
		    	void disconnectNonTransBackupConnection();
		    	void disconnectNonTransConnections();
		    	void disconnectNonTransConnection(const unsigned short index);
		    	void reconnectTransConnection();
		    	void reconnectNonTransConnections();
		    	void reconnectNonTransBackupConnection();
		    	void reconnectNonTransConnection(const unsigned short index);
		    	const unsigned short findNextIdleNonTransConnection();
		    	TissueStackPostgresConnector(
		    			const std::string host,
		    			const short port,
		    			const std::string database,
		    			const std::string user,
		    			const std::string password);
				static TissueStackPostgresConnector * _instance;
				std::vector<pqxx::connection *> _non_trans_connections;
				pqxx::connection * _trans_connection = nullptr;
				pqxx::connection * _non_trans_backup_connection = nullptr;
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
				static const std::vector<const Configuration *> queryAllConfigurations();
				static const bool persistConfiguration(const Configuration * conf);
				static const bool updateConfiguration(const Configuration * conf);
			private:
				static inline Configuration * readResult(pqxx::result::const_iterator result);
		};

		class AtlasInfo final
		{
			public:
				AtlasInfo & operator=(const AtlasInfo&) = delete;
				AtlasInfo(const AtlasInfo&) = delete;
				AtlasInfo(const unsigned long long int id,
					const std::string prefix,
					const std::string description,
					const std::string query_url);
				const unsigned long long int getDataBaseId() const;
				const std::string getPrefix() const;
				const std::string getDescription() const;
				const std::string getQueryUrl() const;
				const std::string toJson() const;
			private:
				const unsigned long long int _id;
				const std::string _prefix;
				const std::string _description;
				const std::string _query_url;
		};

		class SessionDataProvider final
		{
			public:
				SessionDataProvider & operator=(const SessionDataProvider&) = delete;
				SessionDataProvider(const SessionDataProvider&) = delete;
				SessionDataProvider() = delete;
				static const bool addSession(const std::string session, const unsigned long long int expiry_in_millis);
				static const bool hasSessionExpired(
						const std::string session,
						const unsigned long long int now,
						const unsigned long long int extension = 0);
				static const bool invalidateSession(const std::string session);
				static void deleteSessions(const unsigned long long int expiry_in_millis);
		};


		class DataSetDataProvider final
		{
			public:
				static const unsigned short MAX_RECORDS;
				DataSetDataProvider & operator=(const DataSetDataProvider&) = delete;
				DataSetDataProvider(const DataSetDataProvider&) = delete;
				DataSetDataProvider() = delete;
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> queryAll(
						const bool includePlanes = false,
						const unsigned int offset = 0,
						const unsigned int max_records = MAX_RECORDS);
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> queryById(
						const unsigned long long int id,
						const bool includePlanes = false);
				static void findAssociatedDataSets(
						const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData);
				static const bool setIsTiledFlag(const unsigned long long int id, const bool is_tiled);
				static const unsigned short addDataSet(
					const tissuestack::imaging::TissueStackImageData * dataSet,
					const std::string & description);
				static const bool eraseDataSet(const unsigned long long int id);
			private:
				static const std::vector<const tissuestack::imaging::TissueStackImageData *> findResults(
						const std::string sql,
						const unsigned int from = 0,
						const unsigned int to = MAX_RECORDS);
				static void findAndAddPlanes(
						const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData);
				static const std::string SQL;
				static const std::string SQL_PLANES;
				static const std::string SQL_ASSOCIATIONED_SETS;
				static const std::string ORDER_BY;
		};
	}
}

#endif	/* __DATABASE_H__ */
