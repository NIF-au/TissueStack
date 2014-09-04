#include "networking.h"
#include "imaging.h"
#include "database.h"
#include "services.h"

tissuestack::services::TissueStackTilingTask::TissueStackTilingTask(
	const std::string id,
	const std::string input_file,
	const std::string tile_dir,
	const std::vector<std::string> dimensions,
	const std::vector<float> zoom_factors,
	const std::string color_map,
	const unsigned short square,
	const std::string image_format) :
	tissuestack::services::TissueStackTask(id, input_file),
	_tile_dir(tile_dir),
	_dimensions(dimensions),
	_zoom_factors(zoom_factors),
	_color_map(color_map),
	_square(square),
	_image_format(image_format)
{
	if (!this->isInputDataRaw())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Tiling needs raw format data");

	// TODO: more checks
}

tissuestack::services::TissueStackTilingTask::~TissueStackTilingTask() {}

const tissuestack::services::TissueStackTaskType tissuestack::services::TissueStackTilingTask::getType() const
{
	return tissuestack::services::TissueStackTaskType::TILING;
}

const std::string tissuestack::services::TissueStackTilingTask::getParametersForTaskFile() const
{
	// TODO: implement
	return "";
}

void tissuestack::services::TissueStackTilingTask::dumpTaskToDebugLog() const
{
 // TODO: implement
}
