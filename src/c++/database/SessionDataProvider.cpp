#include "database.h"

const bool tissuestack::database::SessionDataProvider::addSession(
	const std::string session, const unsigned long long int expiry_in_millis)
{
	try
	{
		if (session.empty() || expiry_in_millis == 0) return false;

		const std::string sql =
			"INSERT INTO session (id, expiry) VALUES('"
				+ session + "',"
				+ std::to_string(expiry_in_millis)
				+ ");";
		if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
			return true;
	}
	catch(const std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to store session: %s\n", bad.what());
	}
	return false;
}

const bool tissuestack::database::SessionDataProvider::hasSessionExpired(
	const std::string session, const unsigned long long int now, const unsigned long long int extension)
{
	if (session.empty()) return true;

	try
	{
		std::string sql =
			"SELECT * FROM session WHERE id='"
				+ session + "';";
		const pqxx::result result =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);
		if (result.size() != 1) return true;

		const unsigned long long int present_expiry = result[0]["expiry"].as<unsigned long long int>();
		if (present_expiry < now)
		{
			tissuestack::database::SessionDataProvider::invalidateSession(session);
			return true;
		}

		if (extension == 0) return false;

		try
		{
			sql =
				"UPDATE session SET expiry="
					+ std::to_string(now+extension)
					+ " WHERE id='"
					+ session + "';";
			tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql});

			return false;
		} catch(const std::exception & bad)
		{
			// we can ignore that
		}
	}
	catch(const std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to check session expiry: %s\n", bad.what());
	}
	return true;
}

const bool tissuestack::database::SessionDataProvider::invalidateSession(
	const std::string session)
{
	if (session.empty()) return false;

	try
	{
		const std::string sql =
			"DELETE FROM session WHERE id='"
				+ session + "';";
		if (tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql}) == 1)
			return true;
	} catch(const std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to invalidate session: %s\n", bad.what());
	}
	return false;
}

void tissuestack::database::SessionDataProvider::deleteSessions(
		const unsigned long long int expiry_in_millis)
{
	try
	{
		const std::string sql =
			"DELETE FROM session WHERE expiry < "
				+ std::to_string(expiry_in_millis) + ";";
		tissuestack::database::TissueStackPostgresConnector::instance()->executeTransaction({sql});
	} catch(const std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to clean up expired sessions: %s\n", bad.what());
	}
}
