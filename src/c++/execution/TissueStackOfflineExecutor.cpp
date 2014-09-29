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
#include "execution.h"
#include "networking.h"
#include "imaging.h"
#include "services.h"

tissuestack::execution::TissueStackOfflineExecutor::~TissueStackOfflineExecutor()
{
	if (this->_tissueStackRawConverter)
	{
		delete this->_tissueStackRawConverter;
		this->_tissueStackRawConverter = nullptr;
	}

	if (this->_tissueStackPreTiler)
	{
		delete this->_tissueStackPreTiler;
		this->_tissueStackPreTiler = nullptr;
	}
}

tissuestack::execution::TissueStackOfflineExecutor::TissueStackOfflineExecutor()
	: _tissueStackRawConverter(new tissuestack::imaging::RawConverter()),
	  _tissueStackPreTiler(new tissuestack::imaging::PreTiler()) {
	this->setOfflineStrategyFlag();
	this->init();
}

tissuestack::execution::TissueStackOfflineExecutor * tissuestack::execution::TissueStackOfflineExecutor::instance()
{
	if (tissuestack::execution::TissueStackOfflineExecutor::_instance == nullptr)
		tissuestack::execution::TissueStackOfflineExecutor::_instance =
				new tissuestack::execution::TissueStackOfflineExecutor();

	return tissuestack::execution::TissueStackOfflineExecutor::_instance;
}

void tissuestack::execution::TissueStackOfflineExecutor::preTile(
	const std::string & in_file,
	const std::string & tile_dir,
	const std::vector<std::string> & dimensions,
	const std::vector<unsigned short> & zoom_levels,
	const std::string & colormap)
{
	try
	{
		// delegate to PreTiler
		this->_tissueStackPreTiler->preTile(
			this,
			new tissuestack::services::TissueStackTilingTask(
				"0",
				in_file,
				tile_dir,
				dimensions,
				zoom_levels,
				colormap.empty() ? "grey" : colormap,
				256,
				"png"));
	} catch (const std::exception& ex)
	{
		std::cerr << "Failed to pre-tile: " << ex.what() << std::endl;
	}
}

void tissuestack::execution::TissueStackOfflineExecutor::convert(
	const tissuestack::services::TissueStackConversionTask * task,
	const std::string dimension,
	const bool writeHeader)
{
	try
	{
		if (task == nullptr)
			THROW_TS_EXCEPTION(tissuestack::common::TissueStackNullPointerException,
				"I'm missing an instance of a conversion task!");

		// delegate to Converter
		this->_tissueStackRawConverter->convert(this, task, dimension, writeHeader);
	} catch (const std::exception& ex)
	{
		std::cerr << "Failed to convert: " << ex.what() << std::endl;
	}
}
void tissuestack::execution::TissueStackOfflineExecutor::process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// unused for offline
}

void tissuestack::execution::TissueStackOfflineExecutor::init()
{
	this->setRunningFlag(true);
}

void tissuestack::execution::TissueStackOfflineExecutor::stop()
{

	this->raiseStopFlag();
	this->setRunningFlag(false);
}

tissuestack::execution::TissueStackOfflineExecutor * tissuestack::execution::TissueStackOfflineExecutor::_instance = nullptr;

