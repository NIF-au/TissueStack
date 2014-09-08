#include "networking.h"
#include "imaging.h"
#include "services.h"

const std::string tissuestack::networking::TissueStackPreTilingRequest::SERVICE = "TILING";

tissuestack::networking::TissueStackPreTilingRequest::~TissueStackPreTilingRequest()
{
	if (this->_tiling)
		delete this->_tiling;
}

tissuestack::networking::TissueStackPreTilingRequest::TissueStackPreTilingRequest(std::unordered_map<std::string, std::string> & request_parameters) {
	// we need a valid session
	if (tissuestack::services::TissueStackSecurityService::hasSessionExpired(
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "session")))
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Invalid Session! Please Log In.");

	const std::string in_file =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "file");
	if (in_file.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'file' was not supplied!");
	const std::string tile_dir =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"tile_dir");
	if (tile_dir.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'tile_dir' was not supplied!");

	const std::vector<std::string> dimensions =
		tissuestack::utils::Misc::tokenizeString(
			tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(
				tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"dimensions")), ',');
	const std::vector<std::string> zoom_levels =
		tissuestack::utils::Misc::tokenizeString(
			tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(
					tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"zoom")), ',');
	std::vector<unsigned short> numLevels;
	for (auto z : zoom_levels)
		numLevels.push_back(static_cast<unsigned short>(atoi(z.c_str())));
	const std::string tile_size =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"tile_size");

	const std::string colormap =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"color_map");
	const std::string image_format =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"image_type");

	// this will perform more stringent checks
	this->_tiling =
		new tissuestack::services::TissueStackTilingTask(
				tissuestack::services::TissueStackTaskQueue::instance()->generateTaskId(),
			in_file,
			tile_dir,
			dimensions,
			numLevels,
			colormap.empty() ? "grey" : colormap,
			tile_size.empty() ? 256 : static_cast<unsigned int>(atoi(tile_size.c_str())),
			image_format.empty() ? "png" : image_format);

	// we have passed all preliminary checks => assign us the new type
	this->setType(tissuestack::common::Request::Type::TS_TILING);

}
const tissuestack::services::TissueStackTilingTask *
	tissuestack::networking::TissueStackPreTilingRequest::getTask(
			const bool nullOutPointer)
{
	if (!nullOutPointer)
		return this->_tiling;

	const tissuestack::services::TissueStackTilingTask * cpy = this->_tiling;
	this->_tiling = nullptr;

	return cpy;
}

const bool tissuestack::networking::TissueStackPreTilingRequest::isObsolete() const
{
	// a tiling request is regarded obsolete if there is another one running for the same raw file!
	return tissuestack::services::TissueStackTaskQueue::instance()->isBeingTiled(
		this->_tiling->getInputImageData()->getFileName());
}

const std::string tissuestack::networking::TissueStackPreTilingRequest::getContent() const
{
	return std::string("TS_TILING");
}
