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

const std::string tissuestack::database::MetaDataProvider::DATASET_SQL =
	"SELECT PrimaryTable.id, PrimaryTable.filename, PrimaryTable.description, PrimaryTable.lookup_id,"
		" PrimaryTable.is_tiled, PrimaryTable.zoom_levels, PrimaryTable.one_to_one_zoom_level, PrimaryTable.resolution_mm "
		" FROM dataset AS PrimaryTable";
const unsigned short tissuestack::database::MetaDataProvider::MAX_RECORDS = 1000;

const std::vector<const tissuestack::database::DataSetInfo *> tissuestack::database::MetaDataProvider::queryAllDataSets(
		const unsigned int offset, const unsigned int max_records)
{
	const std::string sql =
			tissuestack::database::MetaDataProvider::DATASET_SQL + ";";

	const std::vector<const tissuestack::database::DataSetInfo *> res =
			tissuestack::database::MetaDataProvider::findResults(sql, offset, offset + max_records);

	return res;
}

const tissuestack::database::DataSetInfo * tissuestack::database::MetaDataProvider::queryDataSetInfoById(
		const unsigned long long int id)
{
	const std::string sql =
			tissuestack::database::MetaDataProvider::DATASET_SQL +
			" WHERE PrimaryTable.id=" + std::to_string(id) + " ORDER BY PrimaryTable.id;";

	const std::vector<const tissuestack::database::DataSetInfo *> results =
		tissuestack::database::MetaDataProvider::findResults(sql);

	if (results.size() == 0) return nullptr;
	if (results.size() > 1)
	{
		for (auto r : results)
			delete r;
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");
	}

	return results[0];
}

const std::vector<const tissuestack::database::DataSetInfo *> tissuestack::database::MetaDataProvider::findResults(
		const std::string sql,
		const unsigned int from,
		const unsigned int to)
{
	const pqxx::result results =
			(from == 0 && to == tissuestack::database::MetaDataProvider::MAX_RECORDS) ?
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql) :
			tissuestack::database::TissueStackPostgresConnector::instance()->executePaginatedQuery(sql, from, to);

	if (results.empty())
		return std::vector<const tissuestack::database::DataSetInfo *>();

	std::vector<const tissuestack::database::DataSetInfo *> v_results;
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		// read results and create ImageData
		tissuestack::database::DataSetInfo * rec =
			new tissuestack::database::DataSetInfo(
				i_results["id"].as<unsigned long long int>(),
				i_results["filename"].as<std::string>());
		if (!i_results["description"].is_null())
			rec->setDescription(i_results["description"].as<std::string>());
		rec->setTiled(
			i_results["is_tiled"].as<std::string>() == "T" ? true : false);
		rec->setZoomLevels(i_results["zoom_levels"].as<std::string>());
		rec->setOneToOneZoomLevel(i_results["one_to_one_zoom_level"].as<unsigned int>());
		if (!i_results["resolution_mm"].is_null())
			rec->setResolutionMm(i_results["resolution_mm"].as<float>());

		v_results.push_back(rec);
	}

	return v_results;
}

const bool tissuestack::database::MetaDataProvider::updateDataSetInfo(const DataSetInfo * conf) {

	// TODO: implement
	return true;
}
