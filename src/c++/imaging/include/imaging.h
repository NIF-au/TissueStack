#ifndef	__IMAGE_H__
#define __IMAGE_H__

#include "logging.h"
#include "networking.h"
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <array>
#include <fstream>
#include <unordered_map>

namespace tissuestack
{
	namespace imaging
	{
		/*
		 *             TISSUESTACK RAW FILE HEADER FORMATS
		 *             -----------------------------------
		 *
		 * LEGACY header:
		 * |#DIMS|    DIMS    |      COORDS         |  STEPS    |  DIMS NAME  |DIMS NAME SHORT|
		 *      3|499:1311:679|-124.2:-327.15:-169.2|0.5:0.5:0.5|zspace|yspace|xspace|z|y|x
		 *
		 *LEGACY header (cont'd):
		 * |SLICE SIZE|MAX SLICE SIZE|OFFSETS|ORIG. FORMAT|RAW_FORMAT
		 * |890169:338821:654189|890169|0:1332582993:2665165986|3|2|
		 *
		 *
		 * V1 header:
		 *            |     DIMS   |      COORDS         |   STEPS   |DIMS NAME|ORIG. FORMAT|
		 *             499:1311:679|-124.2:-327.15:-169.2|0.5:0.5:0.5|x:y:z|3|
		 */
		enum RAW_FILE_VERSION
		{
			LEGACY  = 0,
			V1 	= 1,
		};

		enum FORMAT
		{
			MINC    	= 1,	// BACKWARDS COMPATIBILITY FOR MINC
			NIFTI 		= 2,	// BACKWARDS COMPATIBILITY FOR NIFTI
			RAW			= 3,	// TISSUESTACK FORMAT
			DATABASE	= 4		// DATABASE RECORD IMAGE DATA
		};

		enum RAW_TYPE
		{
			UCHAR_8_BIT  	= 1,	// FOR BACKWARDS COMPATIBILITY
			RGB_24BIT		= 2
		};

		class TissueStackLabelLookup final
		{
			public:
				TissueStackLabelLookup & operator=(const TissueStackLabelLookup&) = delete;
				TissueStackLabelLookup(const TissueStackLabelLookup&) = delete;
				~TissueStackLabelLookup();
				static const TissueStackLabelLookup * fromFile(const std::string & filename);
				static const TissueStackLabelLookup * fromDataBaseId(
						const unsigned long long int id,
						const std::string & filename = "",
						const std::string & content = "",
						const tissuestack::database::AtlasInfo * atlasInfo = nullptr);
				const std::string getLabel(const unsigned short & red, const unsigned short & green, const unsigned short & blue) const;
				const std::string getLabelLookupId(bool fullPath=false) const;
				const unsigned long long int getDataBaseId() const;
				void dumpLabelLookupToDebugLog() const;
				void releaseAtlasInfoPointer();
				const std::string toJson() const;
			private:
				const std::string _labellookup_id;
				unsigned long long int _database_id;
				std::array<unsigned short[3], 256> _gray_indexed_rgb_mapping;
				std::unordered_map<std::string, std::string> _label_lookups;
				const tissuestack::database::AtlasInfo * _atlas_info;
				friend class TissueStackColorMap;
				void copyGrayIndexedRgbMapping(std::array<unsigned short[3], 256> & grayIndexedRgbMapping) const;
				friend class tissuestack::database::DataSetDataProvider;
				void setDataBaseInfo(
					const unsigned long long int id,
					const tissuestack::database::AtlasInfo * atlasInfo);
				explicit TissueStackLabelLookup(const std::string & filename);
				explicit TissueStackLabelLookup(
					const unsigned long long int id,
					const std::string & filename,
					const std::string & content,
					const tissuestack::database::AtlasInfo * atlasInfo);
		};

		class TissueStackLabelLookupStore final
		{
			public:
				TissueStackLabelLookupStore & operator=(const TissueStackLabelLookupStore&) = delete;
				TissueStackLabelLookupStore(const TissueStackLabelLookupStore&) = delete;
				static TissueStackLabelLookupStore * instance();
				void purgeInstance();
				const TissueStackLabelLookup * findLabelLookup(const std::string & id) const;
				const TissueStackLabelLookup * findLabelLookupByFullPath(const std::string & id) const;
				const TissueStackLabelLookup * findLabelLookupByDataBaseId(const unsigned long long int id) const;
				void addOrReplaceLabelLookup(const TissueStackLabelLookup * labelLookup);
				const std::unordered_map<std::string, const tissuestack::imaging::TissueStackLabelLookup *> getAllLabelLookups() const;
				void dumpAllLabelLookupsToDebugLog() const;
			private:
				TissueStackLabelLookupStore();
				std::unordered_map<std::string, const TissueStackLabelLookup *> _label_lookups;
				static TissueStackLabelLookupStore * _instance;
		};

		class TissueStackColorMap final
		{
			public:
				TissueStackColorMap & operator=(const TissueStackColorMap&) = delete;
				TissueStackColorMap(const TissueStackColorMap&) = delete;
				static const TissueStackColorMap * fromFile(const std::string & filename);
				static const TissueStackColorMap * fromLabelLookup(const TissueStackLabelLookup * labelLookup);
				static void preFillColorMapArray(std::array<unsigned short[3], 256> & color_map_array);
				const std::array<const unsigned short, 3> getRGBMapForGrayValue(const unsigned short & gray) const;
				const std::string getColorMapId() const;
				void dumpColorMapToDebugLog() const;
				const std::string toJson(bool originalColorMapContents = true) const;
			private:
				const std::string _colormap_id;
				std::array<unsigned short[3], 256> _gray_indexed_rgb_mapping;
				explicit TissueStackColorMap(const std::string & filename);
				explicit TissueStackColorMap(const TissueStackLabelLookup * label_lookup_file);
				void marshallColorMapContentsIntoJson(const std::vector<std::array<float, 4> > & colorMapRanges);
				void marshallLookupFileContentsIntoJson();
				std::string _colorMapFileContentAsJson;
		};

		class TissueStackColorMapStore final
		{
			public:
				TissueStackColorMapStore & operator=(const TissueStackColorMapStore&) = delete;
				TissueStackColorMapStore(const TissueStackColorMapStore&) = delete;
				static TissueStackColorMapStore * instance();
		    	void purgeInstance();
		    	const TissueStackColorMap * findColorMap(const std::string & id) const;
		    	void addOrReplaceColorMap(const TissueStackColorMap * colorMap);
		    	void addOrReplaceColorMap(const TissueStackLabelLookup * labelLookup);
		    	const std::string toJson(bool originalColorMapContents = true) const;
		    	void dumpAllColorMapsToDebugLog() const;
			private:
		    	TissueStackColorMapStore();
		    	std::unordered_map<std::string, const TissueStackColorMap *> _color_maps;
				static TissueStackColorMapStore * _instance;
	 	};

		class TissueStackDataDimension final
		{
			public:
				explicit TissueStackDataDimension(
						const std::string & name,
						const unsigned long long int offset,
						const unsigned long long int number_of_slices,
						const unsigned long long int slice_size);
				explicit TissueStackDataDimension(
						const long long unsigned int id,
						const std::string & name,
						const unsigned long long int number_of_slices);
				TissueStackDataDimension(const TissueStackDataDimension&) = delete;
				TissueStackDataDimension & operator=(const TissueStackDataDimension&) = delete;
				const std::string getName() const;
				const unsigned long long int getNumberOfSlices() const;
				const unsigned long long int getSliceSize() const;
				const unsigned long long int getOffset() const;
				const std::string getTransformationMatrix() const;
				const unsigned int getWidth() const;
				const unsigned int getHeight() const;
				void dumpDataDimensionInfoIntoDebugLog() const;
			private:
				friend class TissueStackImageData;
				void setWidthAndHeight(const std::array<unsigned int, 2> & widthAndHeight);
				void setTransformationMatrix(const std::string transformationMatrix);
				const unsigned long long int _id;
				const std::string 	_name;
				unsigned long long int 	_offset;
				unsigned long long int 	_numberOfSlices;
				unsigned long long int 	_sliceSize;
				unsigned int _width;
				unsigned int _height;
				std::string _transformationMatrix;
		};

		class TissueStackImageData
		{
			public:
				TissueStackImageData & operator=(const TissueStackImageData&) = delete;
				TissueStackImageData(const TissueStackImageData&) = delete;
				static const TissueStackImageData * fromFile(const std::string & filename);
				static const TissueStackImageData * fromDataBaseRecordWithId(
						const unsigned long long int id,
						const bool includePlanes = false);
				virtual ~TissueStackImageData();
				const std::string getFileName() const;
				virtual const bool isRaw() const = 0;
				const FORMAT getFormat() const ;
				const TissueStackDataDimension * getDimension(const char dimension_letter) const;
				const TissueStackDataDimension * getDimensionByLongName(const std::string & dimension) const;
				const std::vector<std::string> getDimensionOrder() const;
				const std::vector<float> getCoordinates() const;
				const std::vector<float> getSteps() const;
				const unsigned short getImageDataMinumum() const;
				const unsigned short getImageDataMaximum() const;
				const unsigned long long int getDataBaseId() const;
				const std::string getDescription() const;
				void addAssociatedDataSet(const TissueStackImageData * associatedDataSet);
				void setMembersFromDataBaseInformation(
						const unsigned long long int id = 0,
						const std::string description = "",
						const bool is_tiled = false,
						const std::vector<float> zoom_levels = {0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2.00},
						const unsigned short one_to_one_zoom_level = 3,
						const float resolution_in_mm = 0,
						const TissueStackLabelLookup * lookup = nullptr);
				const bool isTiled() const;
				const std::vector<float> getZoomLevels() const;
				const unsigned short getOneToOneZoomLevel() const;
				const float getResolutionInMm() const;
				void dumpImageDataIntoDebugLog() const;
				const TissueStackLabelLookup * getLookup() const;
				const int getFileDescriptor();
				void initializeDimensions();
				const std::string toJson(
					const bool includePlanes = false,
					const bool dontDescendIntoAssociated = true) const;
				const std::string getZoomLevelsAsJson() const;
				const bool containsAssociatedDataSet(unsigned long long int dataset_id) const;
				const bool hasZeroDimensions() const;
				const bool hasNoAssociatedDataSets() const;
				void clearAssociatedDataSets();
			protected:
				friend class tissuestack::database::DataSetDataProvider;
				explicit TissueStackImageData(const long long unsigned int id, const std::string filename = "");
				explicit TissueStackImageData(const std::string & filename);
				TissueStackImageData(const std::string & filename, FORMAT format);
				const std::unordered_map<char, const TissueStackDataDimension *> getDimensionMap() const;
				void setFormat(int original_format);
				void addDimension(TissueStackDataDimension * dimension);
				void addCoordinate(float coord);
				void addStep(float step);
			private:
				void setWidthAndHeightByDimension(const std::string & dimension);
				void setTransformationMatrixByDimension(const std::string & dimension);
				void openFileHandle(bool close_open_handle = false);
				void closeFileHandle();
				void dumpDataDimensionInfoIntoDebugLog() const;
				const std::string _file_name;
				std::string _description = "";
				FORMAT _format;
				unsigned short 	_global_min_value = 0;
				unsigned short 	_global_max_value = 255;
				std::vector<std::string> _dim_order;
				std::vector<float> _coordinates;
				std::vector<float> _steps;
				std::unordered_map<char, const TissueStackDataDimension *> _dimensions;
				FILE * _file_handle = nullptr;
				unsigned long long int _database_id = 0;
				bool _is_tiled = false;
				std::vector<float> _zoom_levels = {0.25, 0.5, 0.75, 1, 1.25, 1.5, 1.75, 2.00};
				unsigned short _one_to_one_zoom_level = 3;
				float _resolution_in_mm = 0;
				const TissueStackLabelLookup * _lookup = nullptr;
				std::vector<const TissueStackImageData *> _associated_data_sets;
		};

		class TissueStackRawData final : public TissueStackImageData
		{
			public:
				~TissueStackRawData();
				const bool isRaw() const;
				const unsigned long long int getFileSizeInBytes() const;
				const unsigned long long int getSupposedFileSizeInBytes() const;
				const RAW_TYPE getType() const;
			private:
				void setRawType(int type);
				void setRawVersion(int version);
				friend class TissueStackImageData;
				explicit TissueStackRawData(const std::string & filename);
				void parseHeader(const std::string & header);
				unsigned int _totalHeaderLength = 0;
				unsigned long long int _supposedFileSizeInButes = 0;
				RAW_TYPE	_raw_type = RAW_TYPE::UCHAR_8_BIT;
				RAW_FILE_VERSION _raw_version = RAW_FILE_VERSION::LEGACY;
		};

		class TissueStackDataBaseData final : public TissueStackImageData
		{
			public:
				~TissueStackDataBaseData();
				const bool isRaw() const;
			private:
				friend class tissuestack::database::DataSetDataProvider;
				explicit TissueStackDataBaseData(
						const unsigned long long int id,
						const std::string filename = "");
		};

		class TissueStackNiftiData final : public TissueStackImageData
		{
			public:
				~TissueStackNiftiData();
				const bool isRaw() const;
				const bool isColor();
			private:
				friend class TissueStackImageData;
				TissueStackNiftiData(const std::string & filename);
				bool _is_color = false;
		};

		class TissueStackMincData final : public TissueStackImageData
		{
			public:
				~TissueStackMincData();
				const bool isRaw() const;
			private:
				friend class TissueStackImageData;
				TissueStackMincData(const std::string & filename);
		};

		enum class DataSetStatus
		{
			IN_CONVERSION, READY
		};

		class TissueStackDataSet final
		{
			public:
				TissueStackDataSet & operator=(const TissueStackDataSet&) = delete;
				TissueStackDataSet(const TissueStackDataSet&) = delete;
				~TissueStackDataSet();
				static const TissueStackDataSet * fromFile(const std::string & filename);
				static const TissueStackDataSet * fromTissueStackImageData(const TissueStackImageData * image_data);
				static const TissueStackDataSet * fromDataBaseRecordWithId(
						const unsigned long long id,
						const bool includePlanes = false);
				const TissueStackImageData * getImageData() const;
				const DataSetStatus getStatus() const;
				const std::string getDataSetId() const;
				void dumpDataSetContentIntoDebugLog() const;
				void associateDataSets();
			private:
				DataSetStatus _status = tissuestack::imaging::DataSetStatus::READY;
				const TissueStackImageData * _image_data;
				TissueStackDataSet(const TissueStackImageData * image_data);
		};

		class TissueStackDataSetStore final
		{
			public:
				TissueStackDataSetStore & operator=(const TissueStackDataSetStore&) = delete;
				TissueStackDataSetStore(const TissueStackDataSetStore&) = delete;
				static TissueStackDataSetStore * instance();
				static void integrateDataBaseResultsIntoDataSetStore(
						std::vector<const tissuestack::imaging::TissueStackImageData *> & dataSets);
		    	void purgeInstance();
		    	const TissueStackDataSet * findDataSet(const std::string & id) const;
		    	const TissueStackDataSet * findDataSetByDataBaseId(const unsigned long long int id) const;
		    	void addDataSet(const TissueStackDataSet * dataSet);
		    	void replaceDataSet(const tissuestack::imaging::TissueStackDataSet * dataSet);
		    	void dumpDataSetStoreIntoDebugLog() const;
			private:
		    	TissueStackDataSetStore();
		    	std::unordered_map<std::string, const TissueStackDataSet *> _data_sets;
				static TissueStackDataSetStore * _instance;
	 	};

		class UncachedImageExtraction final
		{
			public:
				UncachedImageExtraction & operator=(const UncachedImageExtraction&) = delete;
				UncachedImageExtraction(const UncachedImageExtraction&) = delete;
				UncachedImageExtraction();

				const std::array<unsigned long long int, 3> performQuery(
					const tissuestack::imaging::TissueStackRawData * image,
					const tissuestack::networking::TissueStackQueryRequest * request) const;

				const Image * extractImage(
					const TissueStackRawData * image,
					const tissuestack::networking::TissueStackImageRequest * request) const;

				Image * extractImageOnly(
					const TissueStackRawData * image,
					const tissuestack::networking::TissueStackImageRequest * request) const;

				Image * applyPostExtractionTasks(
					Image * img,
					const TissueStackRawData * image,
					const tissuestack::networking::TissueStackImageRequest * request) const;

			private:
				void inline changeContrast(
					Image * img,
					const unsigned short minimum,
					const unsigned short maximum,
					const unsigned short dataset_min,
					const unsigned short dataset_max,
					const unsigned long int width,
					const unsigned long int height) const;

				void inline applyColorMap(
					Image * img,
					const std::string color_map_name,
					const unsigned long int width,
					const unsigned long int height) const;

				inline Image * scaleImage(
					Image * img,
					const unsigned int width,
					const unsigned int height) const;

				inline Image * degradeImage(
					Image * img,
					const unsigned int width,
					const unsigned int height,
					const float quality_factor) const;

				inline Image * convertAnythingToRgbImage(Image * img) const;

				inline Image * getImageTile(
					Image * img,
					const tissuestack::networking::TissueStackImageRequest * request) const;

				inline Image * createImageFromDataRead(
					const tissuestack::imaging::TissueStackRawData * image,
					const tissuestack::imaging::TissueStackDataDimension * actualDimension,
					const unsigned char * data) const;

				inline unsigned long long mapUnsignedValue(
					const unsigned char fromBitRange,
					const unsigned char toBitRange,
					const unsigned long long value) const;
		};

		class SimpleCacheHeuristics final
		{
			public:
				SimpleCacheHeuristics & operator=(const SimpleCacheHeuristics&) = delete;
				SimpleCacheHeuristics(const SimpleCacheHeuristics&) = delete;
				SimpleCacheHeuristics();
				explicit SimpleCacheHeuristics(const tissuestack::imaging::UncachedImageExtraction * image_extraction);
				~SimpleCacheHeuristics();

				const Image * extractImage(
					const TissueStackRawData * image,
					const tissuestack::networking::TissueStackImageRequest * request) const;

				const std::array<unsigned long long int, 3> performQuery(
					const tissuestack::imaging::TissueStackRawData * image,
					const tissuestack::networking::TissueStackQueryRequest * request) const;

			private:
				const UncachedImageExtraction * _uncached_extraction = nullptr;
		};

		template <typename CachingStrategy>
		class ImageExtraction final
		{
			public:
				ImageExtraction & operator=(const ImageExtraction&) = delete;
				ImageExtraction(const ImageExtraction&) = delete;
				~ImageExtraction()
				{
					if (this->_caching_strategy)
					{
						delete this->_caching_strategy;
						this->_caching_strategy = nullptr;
					}
				};
				ImageExtraction() : _caching_strategy(new CachingStrategy()) {};

				const TissueStackImageData * processRequest(const tissuestack::networking::TissueStackImageRequest * request,
						const int file_descriptor)
				{
					const tissuestack::imaging::TissueStackDataSet * dataSet =
							tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(request->getDataSetLocation());

					// we have no associated data set, try to create one
					if (dataSet == nullptr)
					{
						std::lock_guard<std::mutex> lock(this->_dataset_addition_mutex);

						try
						{
							dataSet = tissuestack::imaging::TissueStackDataSet::fromFile(request->getDataSetLocation());
							tissuestack::imaging::TissueStackDataSetStore::instance()->addDataSet(dataSet);
						} catch (std::exception & bad)
						{
							tissuestack::logging::TissueStackLogger::instance()->error(
									"Could not create data set from file '%s' for the following reason:\n%s\n",
									request->getDataSetLocation().c_str(), bad.what());
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
									"Given dataset is not a compatible Tissue Stack Data Set!");
						}
					}

					// we only let RAW file requests go through
					if (!dataSet->getImageData()->isRaw())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Only TissueStack Raw Files are allowed to be requested online!");

					const TissueStackDataDimension * dimension  =
							dataSet->getImageData()->getDimensionByLongName(request->getDimensionName());
					if (dimension == nullptr)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"Image Dimension could not be found!");
					if (request->getSliceNumber() < 0 || request->getSliceNumber() > dimension->getNumberOfSlices())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"Slice number requested is out of bounds!");
					if (!request->isPreview()) // only for non preview requests
					{
						if (request->getXCoordinate() < 0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
									"The 'x' (pixel) coordinate has to be a positive integer");
						if (request->getYCoordinate() < 0)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
									"The 'y' (pixel) coordinate has to be a positive integer");
					}

					return dataSet->getImageData();
				}

				void processQueryRequest(
						const tissuestack::networking::TissueStackQueryRequest * request,
						const int file_descriptor)
				{
					const TissueStackImageData * imageData =
							this->processRequest(request, file_descriptor);

					// perform query
					const std::array<unsigned long long int, 3> values =
						this->_caching_strategy->performQuery(
								static_cast<const tissuestack::imaging::TissueStackRawData *>(imageData),
								static_cast<const tissuestack::networking::TissueStackQueryRequest *>(request));

					// start json response
					std::ostringstream response;
					response << "{\"response\": {\""
							<< imageData->getFileName() << "\" : ";

					response << "{\"red\":" << std::to_string(values[0]);
					response << ", \"green\":" << std::to_string(values[1]);
					response << ", \"blue\":" << std::to_string(values[2]) << "}}}";

					const std::string httpResponseHeader =
						tissuestack::utils::Misc::composeHttpResponse(
							"200 OK", "text/json", response.str());
					write(file_descriptor, httpResponseHeader.c_str(), httpResponseHeader.length());
				}

				void processImageRequest(
						const tissuestack::networking::TissueStackImageRequest * request,
						const int file_descriptor)
				{
					const TissueStackImageData * imageData =
						this->processRequest(request, file_descriptor);

					// some more checks regarding the validity of the image request parameters
					if (request->getQualityFactor() <= 0.0 || request->getQualityFactor() > 1.0)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"The range of 'quality factor' has to be greater than 0 but no bigger than 1.0");
					if (request->getScaleFactor() <= 0.0)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"The range of 'scale factor' has to be greater than 0");
					if (request->getColorMapName().empty())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"Request is missing color map information");
					if (tissuestack::imaging::TissueStackColorMapStore::instance()->findColorMap(request->getColorMapName()) == nullptr)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"Request has been given a non-existing color map");
					if (request->getContrastMinimum() < 0 || request->getContrastMinimum() > 255
							|| request->getContrastMaximum() < 0 || request->getContrastMaximum() > 255
							|| request->getContrastMinimum() >= request->getContrastMaximum())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
								"Request has been given invalid contrast parameters");
					if (!request->isPreview()) // only for non preview requests
					{
						if (request->getLengthOfSquare() < 0 || request->getLengthOfSquare() > 256 * 5)
							THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,
									"The length of the image square has to range in betwenn 0 and 1280");
					}

					// perform extraction
					Image * img =
						const_cast<Image *>(
						this->_caching_strategy->extractImage(
								static_cast<const tissuestack::imaging::TissueStackRawData *>(imageData),
								request));

					// timeout check
					if (request->hasExpired())
					{
						DestroyImage(img);
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackObsoleteRequestException,
							"Old Image Request!");
					}

					// this is the part were we start to serialize the output of our finished image work
					std::string formatLowerCase =  request->getOutputImageFormat();
					std::transform(formatLowerCase.begin(), formatLowerCase.end(), formatLowerCase.begin(), tolower);
					strcpy(img->magick, formatLowerCase.c_str());
					std::string image_format("image/");

					// add the header beforehand
					const std::string httpResponseHeader =
							 tissuestack::utils::Misc::composeHttpResponse(
									 "200 OK",
									 image_format + formatLowerCase,
									 ""
					);

					ExceptionInfo exception;
					ImageInfo	*imgInfo = NULL;
					GetExceptionInfo(&exception);
					imgInfo = CloneImageInfo((ImageInfo *)NULL);
					if (imgInfo == NULL)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
								"Could not create ImageInfo!");

					//NOTE: consider gzipped response in the future
					FILE * handle = fdopen(file_descriptor, "w");
					imgInfo->file = handle;
					write(file_descriptor, httpResponseHeader.c_str(), httpResponseHeader.length());
					if (WriteImagesFile(imgInfo, img, handle, &exception) ==0)
						tissuestack::logging::TissueStackLogger::instance()->error(
								"Failed to write out image: %s\n", exception.reason);

					// tidy up
					if (img) DestroyImage(img);
					if (imgInfo) DestroyImageInfo(imgInfo);
					fclose(handle);
				};

			private:
			 	std::mutex _dataset_addition_mutex;
				CachingStrategy * _caching_strategy = nullptr;
		};
	}
}

#endif	/* __IMAGE_H__ */
