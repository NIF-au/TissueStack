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

tissuestack::database::DataSetInfo::DataSetInfo(
		const unsigned long long int id, const std::string filename) :
		_id(id), _filename(filename) {
	if (id < 0 || filename.empty() || filename.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"DataSetInfo Data Object needs a positive id and a non-empty filename!");
}

const unsigned long long int tissuestack::database::DataSetInfo::getId() const
{
	return this->_id;
}

const std::string tissuestack::database::DataSetInfo::getFileName() const
{
	return this->_filename;
}

const std::string tissuestack::database::DataSetInfo::getDescription() const
{
	return this->_description;
}

void tissuestack::database::DataSetInfo::setDescription(const std::string description)
{
	if (description.empty()) return;

	this->_description = description;
}


const std::string tissuestack::database::DataSetInfo::getJson() const
{
	std::ostringstream json;
	json << "{ \"id\": " << std::to_string(this->_id);
	json << ", \"filename\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_filename) << "\"";
	if (!this->_description.empty())
		json << ", \"description\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_description) << "\"";
	json << " }";

	return json.str();
}
