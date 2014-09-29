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

const std::string tissuestack::TissueStackConfigurationParameters::getParameter(const std::string & name) const
{
	const tissuestack::database::Configuration * conf = this->findConfiguration(name);
	if (conf == nullptr) return "";

	return conf->getValue();
}

tissuestack::TissueStackConfigurationParameters * tissuestack::TissueStackConfigurationParameters::instance()
{
	if (tissuestack::TissueStackConfigurationParameters::_instance == nullptr)
		tissuestack::TissueStackConfigurationParameters::_instance =
				new tissuestack::TissueStackConfigurationParameters();

	return tissuestack::TissueStackConfigurationParameters::_instance;
}

const bool tissuestack::TissueStackConfigurationParameters::doesInstanceExist()
{
	return (tissuestack::TissueStackConfigurationParameters::_instance != nullptr);
}

void tissuestack::TissueStackConfigurationParameters::readInConfigurationFile(const std::string & configuration_file)
{
	const std::vector<std::string> lines =
		tissuestack::utils::System::readTextFileLineByLine(configuration_file);
	for (auto line : lines)
	{
		const std::vector<std::string> tokens =
				tissuestack::utils::Misc::tokenizeString(line, '=');
		if (tokens.size() != 2)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Configuration file entries come in pairs of 'name=value'");
		tissuestack::database::Configuration * conf =
			new tissuestack::database::Configuration(tokens[0], tokens[1]);
		this->addOrReplaceConfiguration(conf);
	}
}

inline tissuestack::database::Configuration * tissuestack::TissueStackConfigurationParameters::findConfiguration(const std::string & name) const
{
	try
	{
		return this->_parameters.at(name);
	} catch (std::out_of_range & not_found) {
		return nullptr;
	}
}

void tissuestack::TissueStackConfigurationParameters::addOrReplaceConfiguration(tissuestack::database::Configuration * conf)
{
	if (conf == nullptr) return;

	tissuestack::database::Configuration * existing_conf = this->findConfiguration(conf->getName());
	if (existing_conf == nullptr)
	{
		this->_parameters[conf->getName()] = conf;
		return;
	}

	existing_conf->setValue(conf->getValue());
	delete conf;
}

void tissuestack::TissueStackConfigurationParameters::purgeInstance()
{
	// walk through entries and clean them up
	for (auto entry = this->_parameters.begin(); entry != this->_parameters.end(); ++entry)
		delete entry->second;

	delete tissuestack::TissueStackConfigurationParameters::_instance;
	tissuestack::TissueStackConfigurationParameters::_instance = nullptr;
}

void tissuestack::TissueStackConfigurationParameters::dumpConfigurationToDebugLog() const
{
	tissuestack::logging::TissueStackLogger::instance()->debug("Configuration Parameters:\n");
	for (auto entry = this->_parameters.begin(); entry != this->_parameters.end(); ++entry)
		tissuestack::logging::TissueStackLogger::instance()->debug(
			"Param: %s => %s\n", entry->first.c_str(), entry->second->getValue().c_str());
}

tissuestack::TissueStackConfigurationParameters::TissueStackConfigurationParameters()
{
	this->_parameters["port"] = new tissuestack::database::Configuration("port", "4242");
	this->_parameters["db_host"] = new tissuestack::database::Configuration("db_host", "localhost");
	this->_parameters["db_port"] = new tissuestack::database::Configuration("db_port", "5432");
	this->_parameters["db_name"] = new tissuestack::database::Configuration("db_name", "tissuestack");
	this->_parameters["db_user"] = new tissuestack::database::Configuration("db_user", "tissuestack");
	this->_parameters["db_password"] = new tissuestack::database::Configuration("db_password", "tissuestack");
}


tissuestack::TissueStackConfigurationParameters * tissuestack::TissueStackConfigurationParameters::_instance = nullptr;
