#include "database.h"

const tissuestack::database::Configuration * tissuestack::database::ConfigurationDataProvider::queryConfigurationById(
		const std::string name) {
	if (name.empty())
		return nullptr;


	const std::string sql =
			"SELECT * FROM configuration WHERE name='"
			+ name + "';";

	tissuestack::database::Configuration * ret = nullptr;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	for (pqxx::result::const_iterator conf = results.begin(); conf != results.end(); ++conf)
	{
		ret = tissuestack::database::ConfigurationDataProvider::readResult(conf);
		break;
	}
	tissuestack::logging::TissueStackLogger::instance()->debug("Value:\n%s\n", ret->getValue().c_str());
	return ret;
}

const std::vector<tissuestack::database::Configuration *> tissuestack::database::ConfigurationDataProvider::queryAllConfigurations()
{
	const std::string sql =
			"SELECT * FROM configuration;";

	std::vector<tissuestack::database::Configuration *> ret;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;

	for (pqxx::result::const_iterator conf = results.begin(); conf != results.end(); ++conf)
		ret.push_back(tissuestack::database::ConfigurationDataProvider::readResult(conf));

	return ret;
}

inline tissuestack::database::Configuration * tissuestack::database::ConfigurationDataProvider::readResult(
		pqxx::result::const_iterator result)
{
	return 	new tissuestack::database::Configuration(
			result[0].as<std::string>(),
			result[1].as<std::string>(),
			result[2].is_null() ? "" : result[2].as<std::string>());
}
