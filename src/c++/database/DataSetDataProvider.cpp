#include "database.h"
#include "imaging.h"

const std::string tissuestack::database::DataSetDataProvider::SQL = "SELECT DISTINCT * FROM dataset AS DataSet ";
const std::string tissuestack::database::DataSetDataProvider::ORDER_BY = " ORDER BY DataSet.id ASC";
const unsigned short tissuestack::database::DataSetDataProvider::MAX_RECORDS = 1000;

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryAll(
		const bool includePlanes, const unsigned short offset, const unsigned short max_records)
{
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	return tissuestack::database::DataSetDataProvider::findResults(sql, includePlanes);
}

const tissuestack::imaging::TissueStackImageData * tissuestack::database::DataSetDataProvider::queryById(
		const unsigned long long int id,
		const bool includePlanes)
{
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			" WHERE DataSet.id='" + std::to_string(id) + "'" +
			tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const std::vector<const tissuestack::imaging::TissueStackImageData *> results =
		tissuestack::database::DataSetDataProvider::findResults(sql, includePlanes);

	if (results.size() == 0) return nullptr;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	return results[0];
}

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::findResults(const std::string sql, const bool includePlanes)
{
	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.empty()) return std::vector<const tissuestack::imaging::TissueStackImageData *>();

	std::vector<const tissuestack::imaging::TissueStackImageData *> v_results;
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		// read results and create ImageData
		tissuestack::imaging::TissueStackImageData * rec =
				const_cast<tissuestack::imaging::TissueStackImageData *>(
					tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(
						i_results["DataSet.id"].as<unsigned long long int>()));

		const std::string zoom_levels =
				i_results["DataSet.zoom_levels"].is_null() ? "" :
						i_results["DataSet.zoom_levels"].as<std::string>();
		std::vector<float> v_zoom_levels;

		// TODO: fully set data base info

		rec->setMembersFromDataBaseInformation(
			i_results["DataSet.id"].as<bool>(),
			v_zoom_levels,
			i_results["DataSet.id"].as<unsigned short>(),
			i_results["DataSet.id"].as<float>()
		);

		if (includePlanes)
		{
			// TODO: extract planes info
		}
		v_results.push_back(rec);
	}

	return v_results;
}
