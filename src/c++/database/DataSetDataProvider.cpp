#include "database.h"
#include "imaging.h"

const std::vector<const tissuestack::imaging::TissueStackImageData *> tissuestack::database::DataSetDataProvider::queryAll()
{
	// TODO: implement
	return std::vector<const tissuestack::imaging::TissueStackImageData *>();
}

const tissuestack::imaging::TissueStackImageData * tissuestack::database::DataSetDataProvider::queryById(
		const unsigned long long int id)
{
	// TODO: implement properly
	const std::string sql =
			"SELECT * FROM dataset WHERE id="
			+ std::to_string(id) + ";";

	tissuestack::imaging::TissueStackDataBaseData * ret = new tissuestack::imaging::TissueStackDataBaseData(1);
	ret->addCoordinate(2.3);

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	return ret;
}
