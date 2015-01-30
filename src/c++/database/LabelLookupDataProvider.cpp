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

const tissuestack::imaging::TissueStackLabelLookup * tissuestack::database::LabelLookupDataProvider::queryLookupValuesByFileName(
		const std::string file_name) {
	if (file_name.empty())
		return nullptr;

	const std::string sql =
			"SELECT * FROM dataset_values_lookup WHERE filename='"
			+ file_name + "';";

	tissuestack::imaging::TissueStackLabelLookup * ret = nullptr;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	for (pqxx::result::const_iterator look = results.begin(); look != results.end(); ++look)
	{
		tissuestack::database::AtlasInfo * atlasInfo = nullptr;
		if (!look["atlas_association"].is_null())
		{
			// go query associated atlas info
			const std::string innerSql =
				"SELECT * FROM atlas_info WHERE id=" +
				std::to_string(look["atlas_association"].as<unsigned long long int>()) + ";";

			const pqxx::result innerResults =
					tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(innerSql);

			if (innerResults.size() == 1)
			{
				for (pqxx::result::const_iterator atlas = innerResults.begin(); atlas != innerResults.end(); ++atlas)
				{
					atlasInfo = new tissuestack::database::AtlasInfo(
						atlas["id"].as<unsigned long long int>(),
						atlas["atlas_prefix"].as<std::string>(),
						atlas["atlas_description"].as<std::string>(),
						atlas["atlas_query_url"].is_null() ? "" :
							atlas["atlas_query_url"].as<std::string>());
					break;
				}
			}
		}

		// construct lookup label from db info
		ret = const_cast<tissuestack::imaging::TissueStackLabelLookup * >(
			tissuestack::imaging::TissueStackLabelLookup::fromDataBaseId(
				look["id"].as<unsigned long long int>(),
				look["filename"].as<std::string>(),
				look["content"].as<std::string>(),
				atlasInfo));

		break;
	}
	return ret;
}

const bool tissuestack::database::LabelLookupDataProvider::persistLookupValues(const tissuestack::imaging::TissueStackLabelLookup * lookup)
{
	if (lookup == nullptr) return false;

	// request a new id
	const pqxx::result results =
		tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(
			"SELECT NEXTVAL('atlas_info_id_seq'::regclass);");
	if (results.empty())
		return false;
	const unsigned long long int id =
		results[0][0].as<unsigned long long int>();

	const std::string sql =
		"INSERT INTO dataset_values_lookup (id, filename, content) VALUES("
			+ std::to_string(id)
			+ ",'"
			+ lookup->getLabelLookupId(true) + "','"
			+ lookup->getContentForSql() + "')";
	if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
	{
		const_cast<tissuestack::imaging::TissueStackLabelLookup *>(lookup)->setDataBaseInfo(id, nullptr);
		return true;
	}

	return false;
}

const bool tissuestack::database::LabelLookupDataProvider::updateLookupValues(
		const tissuestack::imaging::TissueStackLabelLookup * hit,
		const tissuestack::imaging::TissueStackLabelLookup * lookup)
{
	if (lookup == nullptr || hit == nullptr) return false;

	tissuestack::database::AtlasInfo * copyOfAtlasInfo = nullptr;
	if (hit->getAtlasInfo() != nullptr)
	{
		copyOfAtlasInfo =
			new tissuestack::database::AtlasInfo(
				hit->getAtlasInfo()->getDataBaseId(),
				hit->getAtlasInfo()->getPrefix(),
				hit->getAtlasInfo()->getDescription(),
				hit->getAtlasInfo()->getQueryUrl());
	}
	const_cast<tissuestack::imaging::TissueStackLabelLookup *>(lookup)->setDataBaseInfo(hit->getDataBaseId(), copyOfAtlasInfo);

	const std::string sql =
		"UPDATE dataset_values_lookup SET content='"
			+ lookup->getContentForSql() + "'"
			+ " WHERE id="
			+ std::to_string(lookup->getDataBaseId()) + ";";
	if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
		return true;

	return false;
}
