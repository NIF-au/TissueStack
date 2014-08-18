#include "database.h"

tissuestack::database::TissueStackPostgresConnector::~TissueStackPostgresConnector()
{
	if (this->_connection)
	{
		if (this->_connection->is_open())
			this->_connection->disconnect();
		delete this->_connection;
		this->_connection = nullptr;
	}
}

tissuestack::database::TissueStackPostgresConnector::TissueStackPostgresConnector(
		const std::string host,
		const short port,
		const std::string password,
		const std::string database,
		const std::string user)
{
	if (host.empty() || password.empty() || database.empty() || user.empty() || port <=0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
						"One or more of the database connection parameters are invalid");

	std::ostringstream connectString;
	connectString << "dbname=" << database << " user=" << user;
	connectString << " host=" << host << " port=" << port;
	connectString << " password=" << password;
	connectString << " sslmode=allow";

	try
	{
		this->_connection = new pqxx::connection(connectString.str());
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

tissuestack::database::TissueStackPostgresConnector * tissuestack::database::TissueStackPostgresConnector::instance(
	const std::string host,
	const short port,
	const std::string password)
 {
	if (tissuestack::database::TissueStackPostgresConnector::_instance == nullptr)
		tissuestack::database::TissueStackPostgresConnector::_instance =
			new tissuestack::database::TissueStackPostgresConnector(
				host, port, password);

	return tissuestack::database::TissueStackPostgresConnector::_instance;
 }

const pqxx::result tissuestack::database::TissueStackPostgresConnector::executeNonTransactionalQuery(const std::string sql)
{
	pqxx::nontransaction non_transaction(*this->_connection);
	return non_transaction.exec(sql);
}

const bool tissuestack::database::TissueStackPostgresConnector::isConnected() const
{
	if (this->_connection == nullptr) return false;
	return this->_connection->is_open();
}

tissuestack::database::TissueStackPostgresConnector * tissuestack::database::TissueStackPostgresConnector::_instance = nullptr;
