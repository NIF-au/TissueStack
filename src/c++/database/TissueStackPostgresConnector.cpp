#include "database.h"
#include "parameters.h"

tissuestack::database::TissueStackPostgresConnector::~TissueStackPostgresConnector()
{
	this->disconnect();
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

	try
	{
		this->_connection = new pqxx::connection(this->_connectString);
	} catch (std::exception & any) {
		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to establish database connection: %s\n", any.what());
	}
}

void tissuestack::database::TissueStackPostgresConnector::purgeInstance()
{
	delete tissuestack::database::TissueStackPostgresConnector::_instance;
	tissuestack::database::TissueStackPostgresConnector::_instance = nullptr;
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
	try
	{
		if (!this->isConnected())
			this->reconnect();

		pqxx::nontransaction non_transaction(*this->_connection);
		return non_transaction.exec(sql);
	} catch (std::exception & bad) { // check connectivity
		if (!this->isConnected()) // we try it one more time
		{
			this->reconnect();
			pqxx::nontransaction non_transaction(*this->_connection);
			return non_transaction.exec(sql);
		}
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute query: %s\n", bad.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failed to execute database query!");
	}
}

const unsigned long long int tissuestack::database::TissueStackPostgresConnector::executeTransaction(const std::vector<std::string> sql)
{
	try
	{
		if (!this->isConnected())
			this->reconnect();

		if (sql.empty()) return 0;

		unsigned long long int affectedRows = 0;

		pqxx::work some_work(*this->_connection);

		pqxx::result result;
		for (auto s : sql)
		{
			tissuestack::logging::TissueStackLogger::instance()->debug("Executing SQL: %s", s.c_str());
			result = some_work.exec(s);
			affectedRows += result.affected_rows();
		}
		some_work.commit();

		return affectedRows;
	} catch (std::exception & bad) { // check connectivity
		if (!this->isConnected()) // we try it one more time
		{
			this->reconnect();
			pqxx::work some_work(*this->_connection);

			unsigned long long int affectedRows = 0;

			pqxx::result result;
			for (auto s : sql)
			{
				result = some_work.exec(s);
				affectedRows += result.affected_rows();
			}

			some_work.commit();

			return affectedRows;
		}
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to execute transaction: %s\n", bad.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Failed to execute database transaction!");
	}
}

const pqxx::result tissuestack::database::TissueStackPostgresConnector::executePaginatedQuery(
		const std::string sql,
		const unsigned int from,
		const unsigned int to)
{
	try
	{
		if (!this->isConnected())
			this->reconnect();

		pqxx::work work( *this->_connection );
		pqxx::stateless_cursor<pqxx::cursor_base::read_only, pqxx::cursor_base::owned> cursor(
				work, sql, "query_cursor", false );

		const pqxx::result res = cursor.retrieve( from, to );
		cursor.close();
		return res;
	} catch (std::exception & bad) { // check connectivity
		if (!this->isConnected()) // we try it one more time
		{
			this->reconnect();
			pqxx::work work( *this->_connection );
			pqxx::stateless_cursor<pqxx::cursor_base::read_only, pqxx::cursor_base::owned> cursor(
					work, sql, "query_cursor", false );

			const pqxx::result res = cursor.retrieve( from, to );
			cursor.close();
			return res;
		}
		throw bad; // propagate (t'was not the connection, strangely enough)
	}
}

void tissuestack::database::TissueStackPostgresConnector::disconnect()
{
	if (this->_connection)
	{
		try {
			if (this->_connection->is_open())
				this->_connection->disconnect();
		} catch(...) {} // anything happens here is disregarded
		delete this->_connection;
		this->_connection = nullptr;
	}
}

void tissuestack::database::TissueStackPostgresConnector::reconnect()
{
	this->disconnect();

	this->_connection = new pqxx::connection(this->_connectString);
}

const bool tissuestack::database::TissueStackPostgresConnector::isConnected() const
{
	if (this->_connection == nullptr) return false;

	try
	{
		return this->_connection->is_open();
	} catch(std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Database not connected for reason: %s\n", bad.what());
		return false;
	}
}

tissuestack::database::TissueStackPostgresConnector * tissuestack::database::TissueStackPostgresConnector::_instance = nullptr;
