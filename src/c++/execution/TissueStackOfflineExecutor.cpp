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
	const std::string & in_file,
	const std::string & out_file)
{
	try
	{
		// delegate to Converter
		this->_tissueStackRawConverter->convert(
			this, new tissuestack::services::TissueStackConversionTask(
				"0",
				in_file,
				out_file));
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

	std::cerr << "\nReceived Crtl + C!" << std::endl;

	this->raiseStopFlag();
	this->setRunningFlag(false);
}

tissuestack::execution::TissueStackOfflineExecutor * tissuestack::execution::TissueStackOfflineExecutor::_instance = nullptr;

