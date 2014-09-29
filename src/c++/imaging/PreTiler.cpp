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

	try
	{
		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task.get()))
		{
			if (processing_strategy->isOnlineStrategy())
				ptr_pretiling_task.release();

			return;
		}

		const std::string fileName = pre_tiling_task->getInputImageData()->getFileName();
		const std::string params = pre_tiling_task->getParametersForTaskFile();
		if (processing_strategy->isOnlineStrategy())
			tissuestack::logging::TissueStackLogger::instance()->info(
				"Starting/Resuming Tiling: %s => %s",
				fileName.c_str(),
				params.c_str());
		else
			std::cout << "Starting Tiling: " << fileName << " => " << params << std::endl;

		this->loopOverDimensions(processing_strategy, ptr_pretiling_task.get());

		if (this->hasBeenCancelledOrShutDown(processing_strategy, ptr_pretiling_task.get()))
		{
			if (processing_strategy->isOnlineStrategy())
				ptr_pretiling_task.release();
			else
				const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);

			return;
		}

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
		{
			std::cout << "Finished Tiling: " << fileName << " => " << params << std::endl;
			const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);
		}
	} catch (const std::exception & bad)
	{
		// flag task as error
		if (processing_strategy->isOnlineStrategy())
		{
			tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to pre-tile: %s => %s",
				pre_tiling_task->getInputImageData()->getFileName().c_str(),
				pre_tiling_task->getParametersForTaskFile().c_str());

			tissuestack::services::TissueStackTaskQueue::instance()->flagTaskAsErroneous(
				ptr_pretiling_task.release()->getId());
		} else
		{
			std::cerr << "Error Tiling: " << bad.what() << std::endl;
			const_cast<tissuestack::common::ProcessingStrategy *>(processing_strategy)->setRunningFlag(false);
		}
	}
}

inline void tissuestack::imaging::PreTiler::loopOverDimensions(
		const tissuestack::common::ProcessingStrategy * processing_strategy,
		const tissuestack::services::TissueStackTilingTask * pretiling_task) const
{
	const std::vector<std::string> dims =
		pretiling_task->getDimensions();

	const std::vector<unsigned short> zoom_levels =
			pretiling_task->getZoomLevels();

	bool resumed = pretiling_task->getSlicesDone() == 0 ? false : true;
	unsigned long long int dimSliceOffset = 0;
	unsigned long long int accDimSliceNumber = 0;

	// loop over all dimension
	for (auto dim : dims)
	{
		// shutdown/cancellation check
		if (this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
			return;

		const tissuestack::imaging::TissueStackDataDimension * actualDimension =
				pretiling_task->getInputImageData()->getDimensionByLongName(dim);

		accDimSliceNumber +=
			(actualDimension->getNumberOfSlices() * static_cast<unsigned long long int>(zoom_levels.size()));

		// resume at a previously interrupted point
		if (resumed && processing_strategy->isOnlineStrategy() &&
			pretiling_task->getSlicesDone() >= accDimSliceNumber)
		{
			dimSliceOffset = accDimSliceNumber;
			continue;
		}

		unsigned long long int zoomSliceOffset = dimSliceOffset;
		unsigned long long int accNumber = zoomSliceOffset;

		// loop over all slices in the dimension
		for (unsigned int sliceNumber = 0; sliceNumber < actualDimension->getNumberOfSlices(); sliceNumber++)
		{
			unsigned long long int accZoomSliceNumber =
				dimSliceOffset +
					static_cast<unsigned long long int>(sliceNumber) *
						static_cast<unsigned long long int>(zoom_levels.size());

			// resume at a previously interrupted point
			if (resumed && processing_strategy->isOnlineStrategy())
			{
				if (pretiling_task->getSlicesDone() >= accZoomSliceNumber)
				{
					zoomSliceOffset = accZoomSliceNumber;
					accNumber = zoomSliceOffset;
					continue;
				}
				if (pretiling_task->getSlicesDone() >= accNumber)
					resumed = false;
			}

			// shutdown/cancellation check
			if (this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
				return;

			Image * img =
				this->_extractor->extractImageForPreTiling(
					static_cast<const tissuestack::imaging::TissueStackRawData *>(pretiling_task->getInputImageData()),
					actualDimension,
					sliceNumber);

			// shutdown/cancellation check
			if (this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
			{
				DestroyImage(img);
				return;
			}

			if (img)
			{
				// now loop over all zoom levels
				for (auto zoom : zoom_levels)
				{
					// resume at a previously interrupted point
					if (resumed && processing_strategy->isOnlineStrategy())
					{
						if (pretiling_task->getSlicesDone() >= accNumber)
						{
							++accNumber;
							continue;
						}
						resumed = false;
					}

					// check/create tile sub directory
					const std::string subDir =
							pretiling_task->getTileDir() + "/" + std::to_string(zoom) +
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

					ExceptionInfo exception;
					GetExceptionInfo(&exception);
					Image * img_processed =
						CloneImage(img, 0, 0, 1, &exception);
					if (img_processed == NULL)
					{
						DestroyImage(img);
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
								"Could not clone image for processsing!");
					}

					img_processed =
						this->_extractor->applyPreTilingProcessing(
							img_processed,
							pretiling_task->getColorMap(),
							width,
							height,
							pretiling_task->getInputImageData()->getZoomLevels()[zoom]
					);

					// shutdown/cancellation check
					if (this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
					{
						DestroyImage(img);
						DestroyImage(img_processed);
						return;
					}

					// chop up into tiles first
					// the x/y loop
					unsigned int upperBoundX =
							(width %
									pretiling_task->getSquareLength() == 0) ?
							(width / pretiling_task->getSquareLength()) :
						ceil(
							static_cast<double>(width) /
							static_cast<double>(pretiling_task->getSquareLength()));
					unsigned int upperBoundY =
							(height %
									pretiling_task->getSquareLength() == 0) ?
							(height / pretiling_task->getSquareLength()) :
						ceil(
							static_cast<double>(height) /
							static_cast<double>(pretiling_task->getSquareLength()));

					for (unsigned int x=0; x<upperBoundX;x++)
					{
						for (unsigned int y=0; y<upperBoundY;y++)
						{
							Image * tile =
								this->_extractor->getImageTileForPreTiling(
									img_processed, x, y, pretiling_task->getSquareLength());
							if (tile)
							{
								this->writeImageToFile(
									tile,
									subDir,
									sliceNumber,
									pretiling_task->getColorMap(),
									false,
									pretiling_task->getImageFormat(),
									x,
									y
								);
							}
						}
					}

					// shutdown/cancellation check
					if (this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
					{
						DestroyImage(img);
						DestroyImage(img_processed);
						return;
					}

					// generate preview last
					img_processed =
						this->_extractor->degradeImage(
							img_processed,
							width,
							height,
							0.1);
					this->writeImageToFile(
						img_processed,
						subDir,
						sliceNumber,
						pretiling_task->getColorMap(),
						true,
						pretiling_task->getImageFormat()
					);

					// now increment slice progress
					if (!this->hasBeenCancelledOrShutDown(processing_strategy, pretiling_task))
					{
						const bool finished =
							const_cast<tissuestack::services::TissueStackTilingTask *>(pretiling_task)->incrementSlicesDone();

						// persist for the online tiling
						if (processing_strategy->isOnlineStrategy())
							tissuestack::services::TissueStackTaskQueue::instance()->persistTaskProgress(
									pretiling_task->getId());
						else
							std::cout << "Progress:\t" <<
								std::to_string(pretiling_task->getSlicesDone()) << "\t["
								<< std::to_string(pretiling_task->getTotalSlices()) << "]\t => "
								<< std::to_string(pretiling_task->getProgress()) << "%\r" << std::flush;

						if (finished)
						{
							if (img) DestroyImage(img);
							return;
						}
					} else
					{
						if (img) DestroyImage(img);
						return;
					}
				}
			}
			if (img) DestroyImage(img);
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
	const tissuestack::services::TissueStackTilingTask * pretiling_task) const
{
	if (processing_strategy == nullptr)
		return true;

	// abortion check
	if (!processing_strategy->isRunning()
			|| processing_strategy->isStopFlagRaised())
		return true;

	// cancel check
	if (pretiling_task == nullptr
		||	(pretiling_task &&
				(pretiling_task->getStatus() ==
						tissuestack::services::TissueStackTaskStatus::CANCELLED
					|| pretiling_task->getStatus() ==
							tissuestack::services::TissueStackTaskStatus::ERRONEOUS)))
		return true;

	return false;
}


