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
