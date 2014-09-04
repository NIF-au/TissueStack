#include "database.h"
#include "networking.h"
#include "imaging.h"

const std::string tissuestack::database::DataSetDataProvider::SQL =
	"SELECT PrimaryTable.id AS prim_id, PrimaryTable.filename, PrimaryTable.description, PrimaryTable.is_tiled,"
		" PrimaryTable.zoom_levels, PrimaryTable.one_to_one_zoom_level, resolution_mm,"
		" SecondaryTable.id AS sec_id, SecondaryTable.filename AS sec_filename, SecondaryTable.content,"
		" TertiaryTable.id AS ter_id, TertiaryTable.atlas_prefix, TertiaryTable.atlas_description, TertiaryTable.atlas_query_url"
		" FROM dataset AS PrimaryTable"
		" LEFT JOIN dataset_values_lookup AS SecondaryTable ON SecondaryTable.id = PrimaryTable.lookup_id"
		" LEFT JOIN atlas_info TertiaryTable ON SecondaryTable.atlas_association = TertiaryTable.id";

const std::string tissuestack::database::DataSetDataProvider::SQL_PLANES =
		"SELECT PrimaryTable.id AS prim_id, PrimaryTable.* FROM dataset_planes AS PrimaryTable ";
const std::string tissuestack::database::DataSetDataProvider::SQL_ASSOCIATIONED_SETS =
		"SELECT PrimaryTable.dataset_id AS prim_id, PrimaryTable.associated_dataset_id AS sec_id"
		" FROM dataset_lookup_mapping AS PrimaryTable ";

const std::string tissuestack::database::DataSetDataProvider::ORDER_BY = " ORDER BY prim_id ASC";
const unsigned short tissuestack::database::DataSetDataProvider::MAX_RECORDS = 1000;

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryAll(
		const bool includePlanes, const unsigned int offset, const unsigned int max_records)
{
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const std::vector<const tissuestack::imaging::TissueStackImageData *> res =
			tissuestack::database::DataSetDataProvider::findResults(sql, offset, offset + max_records);

	if (!includePlanes || res.empty()) return res;

	// integrate dimension/plane data, as well as associated data sets
	for (auto data : res)
	{
		tissuestack::imaging::TissueStackImageData * d = const_cast<tissuestack::imaging::TissueStackImageData *>(data);

		try
		{
			if (data->hasZeroDimensions()) // no need to do this if we have all we need
				tissuestack::database::DataSetDataProvider::findAndAddPlanes(
					data->getDataBaseId(), d);
		} catch (tissuestack::common::TissueStackException & query)
		{
			// if we fail here we have to erase the records
			for (auto sub : res)
				delete sub;
			throw query; // propagate
		}
	}

	return res;
}

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryById(
		const unsigned long long int id,
		const bool includePlanes)
{
	const std::string sql =
			tissuestack::database::DataSetDataProvider::SQL +
			" WHERE PrimaryTable.id=" + std::to_string(id) + " " +
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
	{
		const tissuestack::imaging::TissueStackImageData * hit = results[0];

		try
		{
			if (hit->hasZeroDimensions()) // no need to do this if we have all we need
				tissuestack::database::DataSetDataProvider::findAndAddPlanes(
					hit->getDataBaseId(), const_cast<tissuestack::imaging::TissueStackImageData *>(hit));
		} catch (tissuestack::common::TissueStackException & query)
		{
			// if we fail here we have to erase the record
			delete hit;
			throw query; // propagate
		}
	}

	return results;
}

void tissuestack::database::DataSetDataProvider::findAssociatedDataSets(
		const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData)
{
	if (imageData == nullptr)
		 return;

	const std::string sql =
		tissuestack::database::DataSetDataProvider::SQL_ASSOCIATIONED_SETS +
		" WHERE PrimaryTable.dataset_id=" + std::to_string(dataset_id) + " " +
		tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.empty()) return;

	// loop over results and link data sets
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		unsigned long long int associatedId =
			i_results["sec_id"].as<unsigned long long int>();
		const tissuestack::imaging::TissueStackDataSet * associatedDataSet  =
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSetByDataBaseId(associatedId);

		if (associatedDataSet == nullptr) // last check
		{
			const tissuestack::imaging::TissueStackImageData * associatedImageData =
				tissuestack::imaging::TissueStackImageData::fromDataBaseRecordWithId(associatedId, true);
			if (associatedImageData)
			{
				std::vector<const tissuestack::imaging::TissueStackImageData *> tmp = {associatedImageData};
				tissuestack::imaging::TissueStackDataSetStore::integrateDataBaseResultsIntoDataSetStore(tmp);
				associatedDataSet =
					tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSetByDataBaseId(associatedId);
			}
		}
		if (associatedDataSet) // add if we got something
			imageData->addAssociatedDataSet(associatedDataSet->getImageData());
	}
}

void tissuestack::database::DataSetDataProvider::findAndAddPlanes(
		const unsigned long long int dataset_id, tissuestack::imaging::TissueStackImageData * imageData)
{
	if (imageData == nullptr)
		 return;

	const std::string sql =
		tissuestack::database::DataSetDataProvider::SQL_PLANES +
		" WHERE PrimaryTable.dataset_id=" + std::to_string(dataset_id) + " "
		+ tissuestack::database::DataSetDataProvider::ORDER_BY + ";";

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.empty()) return;

	// loop over planes and create dimension objects
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		tissuestack::imaging::TissueStackDataDimension * rec =
			new tissuestack::imaging::TissueStackDataDimension(
				i_results["prim_id"].as<unsigned long long int>(),
				i_results["name"].as<std::string>(),
				i_results["max_slices"].as<unsigned long long int>());
		rec->setTransformationMatrix(
			i_results["transformation_matrix"].is_null() ? "" : i_results["transformation_matrix"].as<std::string>());
		imageData->addDimension(rec);
	}
	imageData->initializeDimensions(true);
}

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::findResults(
		const std::string sql,
		const unsigned int from,
		const unsigned int to)
{
	const pqxx::result results =
			(from == 0 && to == tissuestack::database::DataSetDataProvider::MAX_RECORDS) ?
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql) :
			tissuestack::database::TissueStackPostgresConnector::instance()->executePaginatedQuery(sql, from, to);

	if (results.empty())
		return std::vector<const tissuestack::imaging::TissueStackImageData *>();

	std::vector<const tissuestack::imaging::TissueStackImageData *> v_results;
	for (pqxx::result::const_iterator i_results = results.begin(); i_results != results.end(); ++i_results)
	{
		// read results and create ImageData
		std::unique_ptr<const tissuestack::imaging::TissueStackImageData> rec(
			new tissuestack::imaging::TissueStackDataBaseData(
				i_results["prim_id"].as<unsigned long long int>(),
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

		// check for lookup info
		std::unique_ptr<const tissuestack::imaging::TissueStackLabelLookup> associatedLookup(nullptr);
		if (!i_results["sec_id"].is_null())
		{
			std::unique_ptr<const tissuestack::database::AtlasInfo> associatedAtlas(nullptr);
			if (!i_results["ter_id"].is_null())
				associatedAtlas.reset(
					new tissuestack::database::AtlasInfo(
						i_results["ter_id"].as<unsigned long long int>(),
						i_results["atlas_prefix"].as<std::string>(),
						i_results["atlas_description"].as<std::string>(),
						i_results["atlas_query_url"].is_null() ? "" :
							i_results["atlas_query_url"].as<std::string>()
					));

			const unsigned long long int lookup_id = i_results["sec_id"].as<unsigned long long int>();
			const std::string label_lookup_file = i_results["sec_filename"].as<std::string>();

			// check if we have already a lookup in memory
			associatedLookup.reset(
				tissuestack::imaging::TissueStackLabelLookupStore::instance()->findLabelLookupByFullPath(label_lookup_file));
			if (associatedLookup.get() == nullptr)
			{
				// try to find it via the database id in memory
				associatedLookup.reset(
					tissuestack::imaging::TissueStackLabelLookupStore::instance()->findLabelLookupByDataBaseId(lookup_id));

				if (associatedLookup.get() == nullptr) // last straw: go to database
				{
					associatedLookup.reset(
						tissuestack::imaging::TissueStackLabelLookup::fromDataBaseId(
							lookup_id,
							label_lookup_file,
							i_results["content"].is_null() ? "" :
								i_results["content"].as<std::string>(),
							associatedAtlas.release()));
					// add it to the lookup store
					tissuestack::imaging::TissueStackLabelLookupStore::instance()->addOrReplaceLabelLookup(associatedLookup.get());
				}
			} else
			{
				const_cast<tissuestack::imaging::TissueStackLabelLookup *>(associatedLookup.get())->setDataBaseInfo(
					lookup_id, associatedAtlas.release());
			}
		}

		const_cast<tissuestack::imaging::TissueStackImageData *>(rec.get())->setMembersFromDataBaseInformation(
			rec->getDataBaseId(),
			i_results["description"].is_null() ? "" : i_results["description"].as<std::string>(),
			i_results["is_tiled"].as<bool>(),
			v_zoom_levels,
			i_results["one_to_one_zoom_level"].as<unsigned short>(),
			i_results["resolution_mm"].is_null() ? 0.0 : i_results["resolution_mm"].as<float>(),
			associatedLookup.release()
		);
		v_results.push_back(rec.release());
	}

	return v_results;
}
