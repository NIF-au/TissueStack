#include "networking.h"
#include "imaging.h"
#include "services.h"

tissuestack::imaging::PreTiler::PreTiler() : _extractor(new tissuestack::imaging::UncachedImageExtraction()) {}

tissuestack::imaging::PreTiler::~PreTiler()
{
	if (this->_extractor)
		delete _extractor;
}

void tissuestack::imaging::PreTiler::preTile(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	const tissuestack::services::TissueStackTilingTask * pre_tiling_task)
{
	// this will be useful for offline tools to not need to keep track of pointer
	std::unique_ptr<const tissuestack::services::TissueStackTilingTask> ptr_pretiling_task(pre_tiling_task);

	// TODO: continue from where we have left off

	try
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		const std::string fileName = pre_tiling_task->getInputImageData()->getFileName();
		const std::string params = pre_tiling_task->getParametersForTaskFile();
		if (processing_strategy->isOnlineStrategy())
			tissuestack::logging::TissueStackLogger::instance()->info(
				"Starting Tiling: %s => %s",
				fileName.c_str(),
				params.c_str());
		else
			std::cout << "Starting Tiling: " << fileName << " => " << params << std::endl;

		this->loopOverDimensions(processing_strategy, ptr_pretiling_task);

		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		// we take care of things in the online/server version
		if (processing_strategy->isOnlineStrategy())
		{
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsFinished(
				ptr_pretiling_task.release()->getId());
			tissuestack::logging::TissueStackLogger::instance()->info(
				"Finished Tiling: %s => %s",
				fileName.c_str(),
				params.c_str());
		} else
			std::cout << "Finished Tiling: " << fileName << " => " << params << std::endl;
	} catch (const std::exception & bad)
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			return;

		tissuestack::logging::TissueStackLogger::instance()->error(
			"Failed to pre-tile: %s => %s",
			pre_tiling_task->getInputImageData()->getFileName().c_str(),
			pre_tiling_task->getParametersForTaskFile().c_str());

		// flag task as error
		if (processing_strategy->isOnlineStrategy())
			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsErroneous(
				ptr_pretiling_task.release()->getId());
	}
}

inline void tissuestack::imaging::PreTiler::loopOverDimensions(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		std::unique_ptr<const tissuestack::services::TissueStackTilingTask> & ptr_pretiling_task) const
{
	const std::vector<std::string> dims =
		ptr_pretiling_task->getDimensions();

	// loop over all dimension
	for (auto dim : dims)
	{
		const tissuestack::imaging::TissueStackDataDimension * actualDimension =
			ptr_pretiling_task->getInputImageData()->getDimensionByLongName(dim);

		// loop over all slices in the dimension
		for (unsigned int sliceNumber = 0; sliceNumber < actualDimension->getNumberOfSlices(); sliceNumber++)
		{
			Image * img =
				this->_extractor->extractImageForPreTiling(
					static_cast<const tissuestack::imaging::TissueStackRawData *>(ptr_pretiling_task->getInputImageData()),
					actualDimension,
					sliceNumber);

			// shutdown/cancellation check
			if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
			{
				DestroyImage(img);
				return;
			}

			if (img)
			{
				// now loop over all zoom levels
				const std::vector<unsigned short> zoom_levels =
					ptr_pretiling_task->getZoomLevels();
				for (auto zoom : zoom_levels)
				{
					// check/create tile sub directory
					const std::string subDir =
						ptr_pretiling_task->getTileDir() + "/" + std::to_string(zoom) +
							"/" + dim.substr(0,1) + "/" + std::to_string(sliceNumber);

					if (!tissuestack::utils::System::directoryExists(subDir) &&
						!tissuestack::utils::System::createDirectory(subDir, 0755))
					{
						DestroyImage(img);
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
								"Could not create tiling sub directory!");
					}

					// apply zoom and colormap if necessary
					unsigned long int width = actualDimension->getWidth();
					unsigned long int height = actualDimension->getHeight();

					img =
						this->_extractor->applyPreTilingProcessing(
							img,
							ptr_pretiling_task->getColorMap(),
							width,
							height,
							ptr_pretiling_task->getInputImageData()->getZoomLevels()[zoom]
					);

					// shutdown/cancellation check
					if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
					{
						DestroyImage(img);
						return;
					}

					// chop up into tiles first
					// the x/y loop
					unsigned int upperBoundX =
							(width %
								ptr_pretiling_task->getSquareLength() == 0) ?
							(width / ptr_pretiling_task->getSquareLength()) :
						ceil(
							static_cast<double>(width) /
							static_cast<double>(ptr_pretiling_task->getSquareLength()));
					unsigned int upperBoundY =
							(height %
								ptr_pretiling_task->getSquareLength() == 0) ?
							(height / ptr_pretiling_task->getSquareLength()) :
						ceil(
							static_cast<double>(height) /
							static_cast<double>(ptr_pretiling_task->getSquareLength()));

					for (unsigned int x=0; x<upperBoundX;x++)
					{
						for (unsigned int y=0; y<upperBoundY;y++)
						{
							Image * tile =
								this->_extractor->getImageTileForPreTiling(
									img, x, y, ptr_pretiling_task->getSquareLength());
							if (tile)
							{
								this->writeImageToFile(
									tile,
									subDir,
									sliceNumber,
									ptr_pretiling_task->getColorMap(),
									false,
									ptr_pretiling_task->getImageFormat(),
									x,
									y
								);
							}
						}
					}

					// shutdown/cancellation check
					if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
					{
						DestroyImage(img);
						return;
					}

					// generate preview last
					img =
						this->_extractor->degradeImage(
							img,
							width,
							height,
							0.1);
					this->writeImageToFile(
						img,
						subDir,
						sliceNumber,
						ptr_pretiling_task->getColorMap(),
						true,
						ptr_pretiling_task->getImageFormat()
					);

					// now increment slice progress
					if (!this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task))
					{
						const bool finished =
							const_cast<tissuestack::services::TissueStackTilingTask *>(ptr_pretiling_task.get())->incrementSlicesDone();

						// persist for the online tiling
						if (processing_strategy->isOnlineStrategy())
							tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(
								ptr_pretiling_task->getId());
						else
							std::cout << "Progress:\t" <<
								std::to_string(ptr_pretiling_task->getSlicesDone()) << "\t["
								<< std::to_string(ptr_pretiling_task->getTotalSlices()) << "]\t => "
								<< std::to_string(ptr_pretiling_task->getProgress()) << "%\r";

						if (finished) return; // this should be kind of superfluous
					}
				}
			}
		}

	}
}

inline void tissuestack::imaging::PreTiler::writeImageToFile(
	Image * img,
	const std::string & tile_dir,
	const unsigned int slice_number,
	const std::string & color_map,
	const bool is_preview,
	const std::string & format,
	const unsigned int x,
	const unsigned int y) const
{
	if (img == NULL) return;

	ExceptionInfo exception;
	ImageInfo	*imgInfo = NULL;
	GetExceptionInfo(&exception);
	imgInfo = CloneImageInfo((ImageInfo *)NULL);
	if (imgInfo == NULL)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"Could not create ImageInfo!");


	std::string formatLowerCase =  format;
	std::transform(formatLowerCase.begin(), formatLowerCase.end(), formatLowerCase.begin(), tolower);
	strcpy(img->magick, formatLowerCase.c_str());

	// assemble file name
	std::ostringstream fileName;

	fileName << tile_dir; // start with root directory
	if (!tile_dir.empty() && tile_dir.at(tile_dir.length()-1) != '/')
		fileName << "/";

	// distinguish between preview and tiles
	if (is_preview) // preview
	{
		fileName << std::to_string(slice_number) << ".low.res.";
		if (!color_map.empty()
			&& color_map.compare("grey") != 0
			&& color_map.compare("gray") != 0)
		{
			fileName << color_map << "."; // append color map
		}
	} else // tile
	{
		fileName << std::to_string(x) << "_"
			<< std::to_string(y);
		if (!color_map.empty()
			&& color_map.compare("grey") != 0
			&& color_map.compare("gray") != 0)
		{
			fileName << "_" << color_map; // append color map
		}
		fileName << ".";
	}
	fileName << formatLowerCase; // finish off with the format

	strcpy(img->filename, fileName.str().c_str());

	if (WriteImage(imgInfo, img) == MagickFail)
	{
		CatchException(&img->exception);
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to write out image: %s\n", img->exception.reason);
	}

	// tidy up
	if (img) DestroyImage(img);
	if (imgInfo) DestroyImageInfo(imgInfo);
}

inline const bool tissuestack::imaging::PreTiler::hasBeenCancelledOrShutDown(
	const tissuestack::common::ProcessingStrategy * processing_strategy,
	std::unique_ptr<const tissuestack::services::TissueStackTilingTask> & ptr_pretiling_task) const
{
	if (processing_strategy == nullptr)
		return true;

	// abortion check
	if (!processing_strategy->isRunning()
			|| processing_strategy->isStopFlagRaised())
	{
		if (processing_strategy->isOnlineStrategy())
			ptr_pretiling_task.release();

		return true;
	}

	// cancel check
	if (ptr_pretiling_task.get() == nullptr
		||	(ptr_pretiling_task.get() != nullptr &&
				(ptr_pretiling_task->getStatus() ==
						tissuestack::services::TissueStackTaskStatus::CANCELLED
					|| ptr_pretiling_task->getStatus() ==
							tissuestack::services::TissueStackTaskStatus::ERRONEOUS)))
	{
		if (processing_strategy->isOnlineStrategy())
			ptr_pretiling_task.release();

		return true;
	}

	return false;
}


