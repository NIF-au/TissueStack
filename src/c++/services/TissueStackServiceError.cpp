#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackServiceError::TissueStackServiceError(
		const std::exception & exception) : _exception(std::string(exception.what())) {}

tissuestack::services::TissueStackServiceError::TissueStackServiceError(
		const tissuestack::common::TissueStackException & exception) : _exception(std::string(exception.what())) {}

const std::string tissuestack::services::TissueStackServiceError::toJson() const
{
	std::ostringstream json;

	json << "{\"error\": { \"exception\": \"";
	std::string sWhat(this->_exception);
	std::string sException = "Exception";
	std::string sDescription = sWhat;

	if (sWhat.empty()) // not specified
	{
		sException = "N/A";
		sDescription = "An unexpected exception without further description occurred";
	} else if (sWhat.at(sWhat.length()-1) == ']')
	{
		// if it is a tissuestack exception we can dissect it into a description and exception part
		size_t iPos = sWhat.rfind("@ LINE:");
		if (iPos != std::string::npos &&
			((iPos = sWhat.rfind("[", iPos)) != std::string::npos))
		{
			sDescription = sWhat.substr(0, iPos-1);
			sException = sWhat.substr(iPos, sWhat.length()-iPos);
		}
	}
	json << tissuestack::utils::Misc::maskQuotesInJson(sException) << "\", \"description\": \"";
	json << tissuestack::utils::Misc::maskQuotesInJson(sDescription) << "\"}}";

	return json.str();
}
