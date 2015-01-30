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
#include "database.h"
#include "parameters.h"

tissuestack::database::TissueStackPostgresConnector::~TissueStackPostgresConnector()
{
	this->disconnectTransConnection();
	this->disconnectNonTransConnections();
}

tissuestack::database::TissueStackPostgresConnector::TissueStackPostgresConnector(
		const std::string host,
		const short port,
		const std::string database,
		const std::string user,
		const std::string password)
{
	if (host.empty() || password.empty() || database.empty() || user.empty() || port <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"One or more of the database connection parameters are invalid");

	std::ostringstream connectString;
	connectString << "dbname=" << database << " user=" << user;
	connectString << " host=" << host << " port=" << port;
	connectString << " password=" << password;
	connectString << " sslmode=allow";
	this->_connectString = connectString.str();

	this->reconnectTransConnection();
	this->reconnectNonTransConnections();
}

void tissuestack::database::TissueStackPostgresConnector::purgeInstance()
{
	delete tissuestack::database::TissueStackPostgresConnector::_instance;
	tissuestack::database::TissueStackPostgresConnector::_instance = nullptr;
}

const bool tissuestack::database::TissueStackPostgresConnector::doesInstanceExist()
{
	return (tissuestack::database::TissueStackPostgresConnector::_instance != nullptr);
}

tissuestack::database::TissueStackPostgresConnector * tissuestack::database::TissueStackPostgresConnector::instance()
 {
	if (tissuestack::database::TissueStackPostgresConnector::_instance == nullptr)
		tissuestack::database::TissueStackPostgresConnector::_instance =
			new tissuestack::database::TissueStackPostgresConnector(
					tissuestack::TissueStackConfigurationParameters::instance()->getParameter("db_host"),
					static_cast<short>(atoi(
							tissuestack::TissueStackConfigurationParameters::instance()->getParameter("db_port").c_str())),
					tissuestack::TissueStackConfigurationParameters::instance()->getParameter("db_name"),
					tissuestack::TissueStackConfigurationParameters::instance()->getParameter("db_user"),
					tissuestack::TissueStackConfigurationParameters::instance()->getParameter("db_password"));

	return tissuestack::database::TissueStackPostgresConnector::_instance;
 }

const pqxx::result tissuestack::database::TissueStackPostgresConnector::executeNonTransactionalQuery(const std::string sql)
{
	//tissuestack::logging::TissueStackLogger::instance()->debug("Executing non transSQL: %s", sql.c_str());

	const unsigned short indexForIdleConnection =
		this->findNextIdleNonTransConnection();

	if (indexForIdleConnection < tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS)
	{
		try
		{
			if (this->_non_trans_connections[indexForIdleConnection] == nullptr)
			{
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
							"Reconnecting database ...");
			}

			pqxx::nontransaction non_transaction(*this->_non_trans_connections[indexForIdleConnection]);

			const pqxx::result ret = non_transaction.exec(sql);

			this->_busyNonTransactionalConnections[indexForIdleConnection] = false;

			return ret;
		} catch (std::exception & bad) { // check connectivity
			this->_busyNonTransactionalConnections[indexForIdleConnection] = false;
			tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
			try
			{
				this->_non_trans_connections[indexForIdleConnection]->activate();
			} catch (...)
			{
				// ignored
			}
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failure to execute database query!");
		}
	}

	// we have reached the maximum of our connection pool =>
	// we switch our strategy to a mutexted backup connection
	{
		std::lock_guard<std::mutex> lock(this->_non_transactionMutex);

		try
		{
			if (this->_non_trans_backup_connection == nullptr)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
							"Reconnecting database ...");

			pqxx::nontransaction non_transaction(*this->_non_trans_backup_connection);

			return non_transaction.exec(sql);
		} catch (std::exception & bad) { // check connectivity
			tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
			try
			{
				this->_non_trans_backup_connection->activate();
			} catch (...)
			{
				// ignored
			}
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failure to execute database query!");
		}
	}

}

const unsigned long long int tissuestack::database::TissueStackPostgresConnector::executeTransaction(const std::vector<std::string> sql)
{
	std::lock_guard<std::mutex> lock(this->_transactionMutex);

	try
	{
		if (sql.empty()) return 0;

		if (this->_trans_connection == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"Reconnecting database ...");

		unsigned long long int affectedRows = 0;

		pqxx::work some_work(*this->_trans_connection);

		pqxx::result result;
		for (auto s : sql)
		{
			//tissuestack::logging::TissueStackLogger::instance()->debug("Executing SQL: %s", s.c_str());
			result = some_work.exec(s);
			affectedRows += result.affected_rows();
		}
		some_work.commit();

		return affectedRows;
	} catch (std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
		try
		{
			this->_trans_connection->activate();
		} catch (...)
		{
			// ignored
		}
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failure to execute database query!");
	}
}

const pqxx::result tissuestack::database::TissueStackPostgresConnector::executePaginatedQuery(
		const std::string sql,
		const unsigned int from,
		const unsigned int to)
{
	const unsigned short indexForIdleConnection =
		this->findNextIdleNonTransConnection();

	if (indexForIdleConnection < tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS)
	{
		try
		{
			if (this->_non_trans_connections[indexForIdleConnection] == nullptr)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Reconnecting database ...");

			pqxx::nontransaction work(*this->_non_trans_connections[indexForIdleConnection]);
			pqxx::stateless_cursor<pqxx::cursor_base::read_only, pqxx::cursor_base::owned> cursor(
					work, sql, "query_cursor", false );

			const pqxx::result res = cursor.retrieve( from, to );
			cursor.close();

			this->_busyNonTransactionalConnections[indexForIdleConnection] = false;

			return res;
		} catch (std::exception & bad) { // check connectivity
			this->_busyNonTransactionalConnections[indexForIdleConnection] = false;
			tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
			try
			{
				this->_non_trans_connections[indexForIdleConnection]->activate();
			} catch (...)
			{
				// ignored
			}
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failure to execute database query!");
		}
	}

	// we have reached the maximum of our connection pool =>
	// we switch our strategy to a mutexted backup connection
	{
		std::lock_guard<std::mutex> lock(this->_non_transactionMutex);

		try
		{
			if (this->_non_trans_backup_connection == nullptr)
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Reconnecting database ...");

			pqxx::nontransaction work(*this->_non_trans_backup_connection);
			pqxx::stateless_cursor<pqxx::cursor_base::read_only, pqxx::cursor_base::owned> cursor(
					work, sql, "query_cursor", false );

			const pqxx::result res = cursor.retrieve( from, to );
			cursor.close();

			return res;
		} catch (std::exception & bad) { // check connectivity
			tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
			try
			{
				this->_non_trans_backup_connection->activate();
			} catch (...)
			{
				// ignored
			}
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Failure to execute database query!");
		}
	}
}

void tissuestack::database::TissueStackPostgresConnector::disconnectTransConnection()
{
	if (this->_trans_connection)
	{
		try {
			if (this->_trans_connection->is_open())
				this->_trans_connection->disconnect();
		} catch(...)
		{
			// anything happens here is disregarded
		}
		delete this->_trans_connection;
		this->_trans_connection = nullptr;
	}
}

void tissuestack::database::TissueStackPostgresConnector::disconnectNonTransBackupConnection()
{
	if (this->_non_trans_backup_connection)
	{
		try {
			if (this->_non_trans_backup_connection->is_open())
				this->_non_trans_backup_connection->disconnect();
		} catch(...)
		{
			// anything happens here is disregarded
		}
		delete this->_non_trans_backup_connection;
		this->_non_trans_backup_connection = nullptr;
	}
}


void tissuestack::database::TissueStackPostgresConnector::disconnectNonTransConnection(
		const unsigned short index)
{
	if (index >= tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS ||
			this->_non_trans_connections[index] == nullptr)
		return;

	this->_busyNonTransactionalConnections[index] = true;

	try {
		if (this->_non_trans_connections[index]->is_open())
			this->_non_trans_connections[index]->disconnect();
	}  catch(...)
	{
		// anything happens here is disregarded
	}

	if (this->_non_trans_connections[index])
	{
		delete this->_non_trans_connections[index];
		this->_non_trans_connections[index] = nullptr;
	}

	this->_busyNonTransactionalConnections[index] = false;
}


void tissuestack::database::TissueStackPostgresConnector::disconnectNonTransConnections()
{
	for (unsigned short index = 0; index < tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS; index++)
		this->disconnectNonTransConnection(index);
	this->disconnectNonTransBackupConnection();
}

void tissuestack::database::TissueStackPostgresConnector::reconnectTransConnection()
{
	this->disconnectTransConnection();

	try
	{
		this->_trans_connection = new pqxx::connection(this->_connectString);
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Could not reconnect database Connection: %s\n", bad.what());
	}
}

void tissuestack::database::TissueStackPostgresConnector::reconnectNonTransConnection(const unsigned short index)
{
	if (index >= tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS)
		return;

	this->disconnectNonTransConnection(index);

	this->_busyNonTransactionalConnections[index] = true;

	try
	{
		this->_non_trans_connections[index] = new pqxx::connection(this->_connectString);
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Could not reconnect database Connection: %s\n", bad.what());
	}

	this->_busyNonTransactionalConnections[index] = false;
}

void tissuestack::database::TissueStackPostgresConnector::reconnectNonTransConnections()
{
	for (unsigned short index = 0; index < tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS; index++)
	{
		this->_busyNonTransactionalConnections[index] = true;
		this->_non_trans_connections.push_back(nullptr);
		this->reconnectNonTransConnection(index);
		this->_busyNonTransactionalConnections[index] = false;
	}
	this->reconnectNonTransBackupConnection();
}

const bool tissuestack::database::TissueStackPostgresConnector::isTransConnected() const
{
	if (this->_trans_connection == nullptr) return false;

	try
	{
		return this->_trans_connection->is_open();
	} catch (pqxx::broken_connection & disconnected)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", disconnected.what());
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", bad.what());
	}
	return false;
}

const bool tissuestack::database::TissueStackPostgresConnector::isNonTransBackupConnected() const
{
	if (this->_non_trans_backup_connection == nullptr) return false;

	try
	{
		return this->_non_trans_backup_connection->is_open();
	} catch (pqxx::broken_connection & disconnected)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", disconnected.what());
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", bad.what());
	}
	return false;
}

void tissuestack::database::TissueStackPostgresConnector::reconnectNonTransBackupConnection()
{
	this->disconnectNonTransBackupConnection();

	try
	{
		this->_non_trans_backup_connection = new pqxx::connection(this->_connectString);
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Could not reconnect database Connection: %s\n", bad.what());
	}
}

const bool tissuestack::database::TissueStackPostgresConnector::isNonTransConnected(const unsigned short index)
{
	if (index >= tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS ||
			this->_non_trans_connections[index] == nullptr)
	{
		this->_busyNonTransactionalConnections[index] = false;
		return false;
	}

	try
	{
		return this->_non_trans_connections[index]->is_open();
	} catch (pqxx::broken_connection & disconnected) {
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", disconnected.what());
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database Connection Error: %s\n", bad.what());
	}
	this->_busyNonTransactionalConnections[index] = false;
	return false;
}

const unsigned short tissuestack::database::TissueStackPostgresConnector::findNextIdleNonTransConnection()
{
	unsigned short index = 0;
	for (index = 0; index < tissuestack::database::TissueStackPostgresConnector::MAX_NON_TRANSACTIONAL_CONNECTIONS; index++)
	{
		if (this->_non_trans_connections[index] != nullptr && !this->_busyNonTransactionalConnections[index])
		{
			this->_busyNonTransactionalConnections[index] = true;
			return index;
		}
	}

	return index;
}


tissuestack::database::TissueStackPostgresConnector * tissuestack::database::TissueStackPostgresConnector::_instance = nullptr;
