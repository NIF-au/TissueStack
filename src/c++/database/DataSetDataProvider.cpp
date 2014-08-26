#include "database.h"
#include "imaging.h"

const std::string tissuestack::database::DataSetDataProvider::SQL = "SELECT * FROM dataset ";
const std::string tissuestack::database::DataSetDataProvider::SQL_PLANES = "SELECT * FROM dataset_planes ";

const std::string tissuestack::database::DataSetDataProvider::ORDER_BY = " ORDER BY id ASC";
const unsigned short tissuestack::database::DataSetDataProvider::MAX_RECORDS = 1000;

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryAll(
		const bool includePlanes, const unsigned short offset, const unsigned short max_records)
{
	// TODO: implement pagination and add associated data set query
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const std::vector<const tissuestack::imaging::TissueStackImageData *> res =
			tissuestack::database::DataSetDataProvider::findResults(sql);

	if (!includePlanes || res.empty()) return res;

	// integrate dimension/plane data
	for (auto data : res)
		tissuestack::database::DataSetDataProvider::findAndAddPlanes(
				data->getDataBaseId(), const_cast<tissuestack::imaging::TissueStackImageData *>(data));

	return res;
}

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryById(
		const unsigned long long int id,
		const bool includePlanes)
{
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			" WHERE id=" + std::to_string(id) + " " +
			tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const std::vector<const tissuestack::imaging::TissueStackImageData *> results =
		tissuestack::database::DataSetDataProvider::findResults(sql);

	if (results.size() == 0) return std::vector<const tissuestack::imaging::TissueStackImageData *>();
	if (results.size() > 1)
	{
		for (auto r : results)
			delete r;
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");
	}

	// integrate dimension/plane data
	if (includePlanes)
		tissuestack::database::DataSetDataProvider::findAndAddPlanes(
			results[0]->getDataBaseId(), const_cast<tissuestack::imaging::TissueStackImageData *>(results[0]));

	return results;
}

void tissuestack::database::DataSetDataProvider::findAndAddPlanes(
		const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData)
{
	if (imageData == nullptr)
		 return;

	const std::string sql =
		tissuestack::database::DataSetDataProvider::SQL_PLANES +
		" WHERE dataset_id=" + std::to_string(dataset_id) + " " + tissuestack::database::DataSetDataProvider::ORDER_BY;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.empty()) return;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	// loop over planes and create dimension objects
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		tissuestack::imaging::TissueStackDataDimension * rec =
			new tissuestack::imaging::TissueStackDataDimension(
				i_results["name"].as<std::string>(),
				i_results["max_slices"].as<unsigned long long int>());
		imageData->addDimension(rec);
	}
	imageData->initializeWidthAndHeightForDimensions();
}

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::findResults(const std::string sql)
{
	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.empty())
		return std::vector<const tissuestack::imaging::TissueStackImageData *>();

	std::vector<const tissuestack::imaging::TissueStackImageData *> v_results;
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		// read results and create ImageData
		std::unique_ptr<const tissuestack::imaging::TissueStackImageData> rec(
			new tissuestack::imaging::TissueStackDataBaseData(
				i_results["id"].as<unsigned long long int>(),
				i_results["filename"].as<std::string>()));

		std::string zoom_levels =
				i_results["zoom_levels"].is_null() ? "" :
						i_results["zoom_levels"].as<std::string>();

		zoom_levels = tissuestack::utils::Misc::eraseCharacterFromString(zoom_levels, '[');
		zoom_levels = tissuestack::utils::Misc::eraseCharacterFromString(zoom_levels, ']');
		zoom_levels = tissuestack::utils::Misc::eraseCharacterFromString(zoom_levels, ' ');
		std::vector<float> v_zoom_levels;
		const std::vector<std::string> s_zoom_levels =
				tissuestack::utils::Misc::tokenizeString(zoom_levels, ',');
		for (auto z : s_zoom_levels)
			v_zoom_levels.push_back(static_cast<float>(atof(z.c_str())));

		const_cast<tissuestack::imaging::TissueStackImageData *>(rec.get())->setMembersFromDataBaseInformation(
			i_results["description"].is_null() ? "" : i_results["description"].as<std::string>(),
			i_results["is_tiled"].as<bool>(),
			v_zoom_levels,
			i_results["one_to_one_zoom_level"].as<unsigned short>(),
			i_results["resolution_mm"].is_null() ? 0.0 : i_results["resolution_mm"].as<float>()
		);
		v_results.push_back(rec.release());
	}

	return v_results;
}
