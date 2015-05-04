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
#include "networking.h"
#include "imaging.h"

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
	return ret;
}

const bool tissuestack::database::ConfigurationDataProvider::persistConfiguration(const tissuestack::database::Configuration * conf)
{
	if (conf == nullptr) return false;

	const std::string sql =
		"INSERT INTO configuration (name, value, description) VALUES('"
			+ conf->getName() + "','"
			+ conf->getValue() + "', "
			+ (conf->getDescription().empty() ? "NULL" : ("'" + conf->getDescription() + "'"))
			+ ");";
	if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
		return true;

	return false;
}

const bool tissuestack::database::ConfigurationDataProvider::updateConfiguration(const tissuestack::database::Configuration * conf)
{
	if (conf == nullptr) return false;

	const std::string sql =
		"UPDATE configuration SET value='"
			+ conf->getValue() + "'"
			+ " WHERE name='"
			+ conf->getName() + "';";
	if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
		return true;

	return false;
}

const std::vector<const tissuestack::database::Configuration *> tissuestack::database::ConfigurationDataProvider::queryAllConfigurations()
{
	const std::string sql =
			"SELECT * FROM configuration;";

	std::vector<const tissuestack::database::Configuration *> ret;

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
			result["name"].as<std::string>(),
			result["value"].as<std::string>(),
			result["description"].is_null() ? "" : result["description"].as<std::string>());
}

const std::string tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory(const std::string which)
{
	if (which.empty())
		return "";

	tissuestack::database::Configuration * dirConf = nullptr;
	try
	{
		dirConf =
			const_cast<tissuestack::database::Configuration *>(
				tissuestack::database::ConfigurationDataProvider::queryConfigurationById(which));
	} catch (std::exception & any ) {
		// ignored
	}
	if (dirConf == nullptr)
		return "";

	const std::string val = dirConf->getValue();
	delete dirConf;

	return val;
}

const std::string tissuestack::database::ConfigurationDataProvider::getTemporaryDirectory()
{
	std::string tmpDir =
		tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory("temp_directory");

	if (tmpDir.empty())
		tmpDir = TISSUESTACK_TMP_DIR;

	return tmpDir;
}
