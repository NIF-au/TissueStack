#include "database.h"

tissuestack::database::Configuration::Configuration(
		const std::string name, const std::string value, const std::string description) :
		_name(name), _value(value), _description(description) {
	if (name.empty() || value.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
								"Configuration Data Object needs a non-empty name/value pair!");
}

const std::string tissuestack::database::Configuration::getName() const
{
	return this->_name;
}

const std::string tissuestack::database::Configuration::getValue() const
{
	return this->_value;
}

const std::string tissuestack::database::Configuration::getDescription() const
{
	return this->_description;
}

void tissuestack::database::Configuration::setValue(const std::string value)
{
	if (value.empty()) return;

	this->_value = value;
}


const std::string tissuestack::database::Configuration::getJson() const
{
	std::ostringstream json;
	json << "{ \"name\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_name) << "\"";
	json << ", \"value\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_value) << "\"";
	if (!this->_description.empty())
		json << ", \"description\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_description) << "\"";
	json << " }";

	return json.str();
}
