#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

#include <openssl/sha.h>

const std::string tissuestack::services::TissueStackSecurityService::SUB_SERVICE_ID = "SECURITY";
const std::string tissuestack::services::TissueStackSecurityService::DEFAULT_GLOBAL_ADMIN_PASSWORD_AS_SHA_2_HEX_STRING =
		"101ee9fe7aceaa8bea949e75a529d796da02e08bced78c6c4dde60768183fa14";
const unsigned long long int tissuestack::services::TissueStackSecurityService::DEFAULT_SESSION_TIMEOUT = 1000 * 60 * 15; // 15 minutes of inactivity

tissuestack::services::TissueStackSecurityService::TissueStackSecurityService() {
	this->addMandatoryParametersForRequest("SHA2_HASH",
		std::vector<std::string>{ "EXPRESSION"});
	this->addMandatoryParametersForRequest("PASSWD",
		std::vector<std::string>{ "OLD_PASSWD", "NEW_PASSWD"});
	this->addMandatoryParametersForRequest("NEW_SESSION",
		std::vector<std::string>{ "PASSWORD"});
	this->addMandatoryParametersForRequest("CHECK_SESSION",
		std::vector<std::string>{ "SESSION"});
	this->addMandatoryParametersForRequest("INVALIDATE_SESSION",
		std::vector<std::string>{ "SESSION"});
};

tissuestack::services::TissueStackSecurityService::~TissueStackSecurityService() {};

void tissuestack::services::TissueStackSecurityService::checkRequest(
		const tissuestack::networking::TissueStackServicesRequest * request) const
{
	this->checkMandatoryRequestParameters(request);
}

void tissuestack::services::TissueStackSecurityService::streamResponse(
		const tissuestack::networking::TissueStackServicesRequest * request,
		const int file_descriptor) const
{
	const std::string action = request->getRequestParameter("ACTION", true);

	std::ostringstream json;

	if (action.compare("SHA2_HASH") == 0)
	{
		json << "{ \"response\": \""
			<< this->encodeSHA256(request->getRequestParameter("EXPRESSION"))
			<< "\"}";
	} else if (action.compare("PASSWD") == 0)
	{
		if (!tissuestack::services::TissueStackSecurityService::isAdminPassword(
			request->getRequestParameter("OLD_PASSWD")))
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"The given password does not match your existing password!");

		if (!tissuestack::services::TissueStackSecurityService::setNewAdminPassword(
			request->getRequestParameter("NEW_PASSWD")))
				THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
					"Could not set new password!");
		json << "{\"response\":\"Password Changed\"}";
	} else if (action.compare("NEW_SESSION") == 0)
	{
		if (!tissuestack::services::TissueStackSecurityService::isAdminPassword(
			request->getRequestParameter("password")))
			json << tissuestack::common::NO_RESULTS_JSON;
		else {
			const std::string sessionToken =
				tissuestack::utils::System::generateUUID();
			const unsigned long long int expiry =
				tissuestack::utils::System::getSystemTimeInMillis() +
				tissuestack::services::TissueStackSecurityService::DEFAULT_SESSION_TIMEOUT;

			if (!tissuestack::database::SessionDataProvider::addSession(sessionToken, expiry))
				json << tissuestack::common::NO_RESULTS_JSON;
			else
			{
				json << "{\"response\": {\"expiry\":"
					<< std::to_string(expiry)
					<< ", \"id\": \""
					<< sessionToken << "\"}}";
			}
		}
	} else if (action.compare("CHECK_SESSION") == 0)
	{
		if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
			request->getRequestParameter("SESSION")))
		{
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Invalid Session! Please Log In.");
		} else
			json << tissuestack::common::NO_RESULTS_JSON;

	} else if (action.compare("INVALIDATE_SESSION") == 0)
	{
		if (tissuestack::database::SessionDataProvider::invalidateSession(
			request->getRequestParameter("SESSION")))
			json << "{\"response\": \"Session invalidated\"}";
		else
			json << tissuestack::common::NO_RESULTS_JSON;
	}

	const std::string response =
			tissuestack::utils::Misc::composeHttpResponse("200 OK", "application/json", json.str());
	write(file_descriptor, response.c_str(), response.length());
}

const bool tissuestack::services::TissueStackSecurityService::isAdminPassword(const std::string & password) const
{
	if (password.empty()) return false;

	const std::string encodedPassword = tissuestack::services::TissueStackSecurityService::encodeSHA256(password);

	const tissuestack::database::Configuration * passwd =
			tissuestack::database::ConfigurationDataProvider::queryConfigurationById("admin_passwd");
	 // if we have no password in the data base => use default
	std::string wantedAdminPassword =
		tissuestack::services::TissueStackSecurityService::DEFAULT_GLOBAL_ADMIN_PASSWORD_AS_SHA_2_HEX_STRING;
	if (passwd)
	{
		wantedAdminPassword = passwd->getValue();
		delete passwd;
	}
	// compare
	if (wantedAdminPassword.compare(encodedPassword) == 0)
		return true;

	return false;
}

const bool tissuestack::services::TissueStackSecurityService::setNewAdminPassword(const std::string & password) const
{
	if (password.empty()) return false;

	try
	{
		// look existing password up
		std::unique_ptr<tissuestack::database::Configuration> passwd(
			const_cast<tissuestack::database::Configuration *>(
				tissuestack::database::ConfigurationDataProvider::queryConfigurationById("admin_passwd")));

		if (passwd.get() == nullptr) // it does not exist => we create it and store it
		{
			passwd.reset(
				new tissuestack::database::Configuration(
					"admin_passwd",
					tissuestack::services::TissueStackSecurityService::encodeSHA256(password),
					"admin password"));
			if (tissuestack::database::ConfigurationDataProvider::persistConfiguration(passwd.get()))
				return true;
		} else {
			passwd->setValue(
				tissuestack::services::TissueStackSecurityService::encodeSHA256(password));
			if (tissuestack::database::ConfigurationDataProvider::updateConfiguration(passwd.get()))
				return true;
		}
	} catch(const std::exception & bad)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to set new password: %s\n", bad.what());
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Could not set new password!");
	}
	return false;
}

const bool tissuestack::services::TissueStackSecurityService::hasSessionExpired(const std::string session)
{
	return tissuestack::database::SessionDataProvider::hasSessionExpired(
			session,
			tissuestack::utils::System::getSystemTimeInMillis(),
			tissuestack::services::TissueStackSecurityService::DEFAULT_SESSION_TIMEOUT);
}

const std::string tissuestack::services::TissueStackSecurityService::encodeSHA256(const std::string & expression) const
{
	unsigned char hash[SHA256_DIGEST_LENGTH];

	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, expression.c_str(), expression.size());
	SHA256_Final(hash, &sha256);

	std::stringstream stream;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(hash[i]);
	}

	return stream.str();
}
