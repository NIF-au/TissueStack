#include "database.h"

tissuestack::database::AtlasInfo::AtlasInfo(
	const unsigned long long int id,
	const std::string prefix,
	const std::string description,
	const std::string query_url) :
		_id(id), _prefix(prefix), _description(description), _query_url(query_url) {}

const unsigned long long int tissuestack::database::AtlasInfo::getDataBaseId() const
{
	return this->_id;
}

const std::string tissuestack::database::AtlasInfo::getPrefix() const
{
	return this->_prefix;
}

const std::string tissuestack::database::AtlasInfo::getDescription() const
{
	return this->_description;
}

const std::string tissuestack::database::AtlasInfo::getQueryUrl() const
{
	return this->_query_url;
}


const std::string tissuestack::database::AtlasInfo::toJson() const
{
	std::ostringstream json;
	json << "{ \"id\": " << std::to_string(this->_id);
	json << ", \"prefix\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_prefix) << "\"";
	json << ", \"description\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_description) << "\"";
	if (!this->_query_url.empty())
		json << ", \"queryUrl\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_query_url) << "\"";
	json << " }";

	return json.str();
}
