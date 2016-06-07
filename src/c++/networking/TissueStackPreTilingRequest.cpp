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
#include "networking.h"
#include "imaging.h"
#include "database.h"
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
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"Invalid Session! Please Log In.");

	std::string in_file =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "file");
	const std::string id =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters, "id");
	if (id.empty() && in_file.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Mandatory parameter 'file' or 'id' was not supplied!");
	const tissuestack::imaging::TissueStackDataSet * foundDataSet =
		!in_file.empty() ?
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(in_file) :
			tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSetByDataBaseId(
				strtoull(id.c_str(), NULL, 10));
	if (foundDataSet == nullptr || foundDataSet->getImageData()->getDataBaseId() == 0)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
			"Could not find dataset for given file/id!");
	if (in_file.empty())
		in_file = foundDataSet->getDataSetId();
	std::string tile_dir =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"tile_dir");
	if (tile_dir.empty()) {
		tile_dir =
			tissuestack::database::ConfigurationDataProvider::findSpecificApplicationDirectory("server_tile_directory");
		if (tile_dir.empty())
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
				"Mandatory parameter 'tile_dir' was not supplied!");
	}
	tile_dir = tile_dir + "/" + std::to_string(foundDataSet->getImageData()->getDataBaseId());

	const std::vector<std::string> dimensions =
		tissuestack::utils::Misc::findUnorderedMapEntryWithUpperCaseStringKey(request_parameters,"dimensions").compare("*") == 0 ?
			foundDataSet->getImageData()->getDimensionOrder() :
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
	if (numLevels.empty()) {
		for (unsigned short c = 0;c<foundDataSet->getImageData()->getZoomLevels().size();c++) {
			numLevels.push_back(c);
		}
	}

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
	return tissuestack::services::TissueStackTaskQueue::instance()->isBeingTiled(this->_tiling);
}

const std::string tissuestack::networking::TissueStackPreTilingRequest::getContent() const
{
	return std::string("TS_TILING");
}
