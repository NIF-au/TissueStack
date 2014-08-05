#ifndef	__IMAGE_H__
#define __IMAGE_H__

#include "logging.h"
#include "networking.h"
#include <memory.h>
#include <array>
#include <fstream>
#include <unordered_map>

namespace tissuestack
{
	namespace imaging
	{
		enum class FORMAT
		{
			MINC    = 1,	// BACKWARDS COMPATIBILITY FOR MINC
			NIFTI 	= 2,	// BACKWARDS COMPATIBILITY FOR NIFTI
			RAW		= 3
		};

		enum class RAW_TYPE
		{
			UCHAR_8_BIT  	= 1,	// FOR BACKWARDS COMPATIBILITY
			RGB_24BIT		= 2
		};

		class TissueStackLabelLookup final
		{
			public:
				TissueStackLabelLookup & operator=(const TissueStackLabelLookup&) = delete;
				TissueStackLabelLookup(const TissueStackLabelLookup&) = delete;
				static const TissueStackLabelLookup * fromFile(const std::string & filename);
				void copyGrayIndexedRgbMapping(std::array<unsigned short[3], 256> & grayIndexedRgbMapping) const;
				const std::string getLabel(const unsigned short & red, const unsigned short & green, const unsigned short & blue) const;
				const std::string getLabelLookupId() const;
				void dumpLabelLookupToDebugLog() const;
			private:
				const std::string _labellookup_id;
				std::array<unsigned short[3], 256> _gray_indexed_rgb_mapping;
				std::unordered_map<std::string, std::string> _label_lookups;
				explicit TissueStackLabelLookup(const std::string & filename);
		};

		class TissueStackLabelLookupStore final
		{
			public:
				TissueStackLabelLookupStore & operator=(const TissueStackLabelLookupStore&) = delete;
				TissueStackLabelLookupStore(const TissueStackLabelLookupStore&) = delete;
				static TissueStackLabelLookupStore * instance();
				void purgeInstance();
				const TissueStackLabelLookup * findLabelLookup(const std::string & id) const;
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
			private:
				const std::string _colormap_id;
				std::array<unsigned short[3], 256> _gray_indexed_rgb_mapping;
				explicit TissueStackColorMap(const std::string & filename);
				explicit TissueStackColorMap(const TissueStackLabelLookup * label_lookup_file);
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
		    	void dumpAllColorMapsToDebugLog() const;
			private:
		    	TissueStackColorMapStore();
		    	std::unordered_map<std::string, const TissueStackColorMap *> _color_maps;
				static TissueStackColorMapStore * _instance;
	 	};

		class TissueStackDataDimension final
		{
			public:
				explicit TissueStackDataDimension(const std::string & name);
				TissueStackDataDimension(const TissueStackDataDimension&) = delete;
				TissueStackDataDimension & operator=(const TissueStackDataDimension&) = delete;
				const std::string getName() const;
				const unsigned long long int getNumberOfSlices() const;
				const unsigned long long int getOffset() const;
				const int getMinumum() const;
				const int getMaximum() const;

			private:
				const std::string 	_name;
				unsigned long long int 	_slices = 0;
				unsigned long long int 	_offset = 0;
				int 	_min_value = -1;
				int 	_max_value = -1;

		};

		class TissueStackImageData
		{
			public:
				TissueStackImageData & operator=(const TissueStackImageData&) = delete;
				TissueStackImageData(const TissueStackImageData&) = delete;
				static const TissueStackImageData * fromFile(const std::string & filename);
				virtual ~TissueStackImageData();
				const std::string getFileName() const;
				virtual const bool isRaw() = 0;
				const FORMAT getFormat() const ;
				const TissueStackDataDimension * getDimension(const char dimension_letter) const;
				const TissueStackDataDimension * getDimensionByLongName(const std::string & dimension) const;
				const int getGlobalMinumum() const;
				const int getGlobalMaximum() const;
			protected:
				explicit TissueStackImageData(const std::string & filename);
				TissueStackImageData(const std::string & filename, const FORMAT format);
				void closeFileHandle();
			private:
				const std::string		_file_name;
				FILE *			_file_handle = nullptr;
				const FORMAT _format;
				long long int 	_global_min_value = -1;
				long long int 	_global_max_value = -1;
				std::unordered_map<char, const TissueStackDataDimension *> _dimensions;
		};

		class TissueStackRawData final : public TissueStackImageData
		{
			public:
				const bool isRaw();
			private:
				friend class TissueStackImageData;
				explicit TissueStackRawData(const std::string & filename);
				const RAW_TYPE	_raw_type = RAW_TYPE::RGB_24BIT;
		};

		class TissueStackNiftiData final : public TissueStackImageData
		{
			public:
				~TissueStackNiftiData();
				const bool isRaw();
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
				const bool isRaw();
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
				const TissueStackImageData * getImageData() const;
				const DataSetStatus getStatus() const;
				const std::string getDataSetId() const;
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
		    	void purgeInstance();
		    	const TissueStackDataSet * findDataSet(const std::string & id) const;
		    	void addOrReplaceDataSet(const TissueStackDataSet * dataSet);
			private:
		    	TissueStackDataSetStore();
		    	std::unordered_map<std::string, const TissueStackDataSet *> _data_sets;
				static TissueStackDataSetStore * _instance;
	 	};

		class SimpleCacheHeuristics final
		{
			public:
				SimpleCacheHeuristics & operator=(const SimpleCacheHeuristics&) = delete;
				SimpleCacheHeuristics(const SimpleCacheHeuristics&) = delete;
				SimpleCacheHeuristics();
				~SimpleCacheHeuristics();

				// TODO: set appropriate return type, e.g. graphicsmagick (perhaps wrap to be something more generic)
				void extractImage(
						const TissueStackImageData * image,
						const TissueStackDataDimension * dimension,
						const unsigned long long int slice);
		};

		class UncachedImageExtraction final
		{
			public:
				UncachedImageExtraction & operator=(const UncachedImageExtraction&) = delete;
				UncachedImageExtraction(const UncachedImageExtraction&) = delete;
				UncachedImageExtraction();

				// TODO: set appropriate return type, e.g. graphicsmagick (perhaps wrap to be something more generic)
				void extractImage(
						const TissueStackImageData * image,
						const TissueStackDataDimension * dimension,
						const unsigned long long int slice);

				void extractImages(
						const tissuestack::networking::TissueStackImageRequest * request,
						const int file_descriptor);

			private:
				SimpleCacheHeuristics _source;
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

				void processImageRequest(
						const tissuestack::networking::TissueStackImageRequest * request,
						const int file_descriptor)
				{
					const tissuestack::imaging::TissueStackDataSet * dataSet =
							tissuestack::imaging::TissueStackDataSetStore::instance()->findDataSet(request->getDataSetLocation());

					// we have no associated data set, try to create one
					if (dataSet == nullptr)
					{
						 std::lock_guard<std::mutex> lock(this->_dataset_addition_mutex);

						 dataSet = tissuestack::imaging::TissueStackDataSet::fromFile(request->getDataSetLocation());
						 tissuestack::imaging::TissueStackDataSetStore::instance()->addOrReplaceDataSet(dataSet);
					}

					// some more checks regarding the validity of the image request parameters
					const tissuestack::imaging::TissueStackDataDimension * dimension  =
							dataSet->getImageData()->getDimensionByLongName(request->getDimensionName());
					if (dimension == nullptr)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Image Dimension could not be found!");
					if (request->getSliceNumber() < 0 || request->getSliceNumber() > dimension->getNumberOfSlices())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "Slice number requested is out of bounds!");

					if (request->getLengthOfSquare() < 0 || request->getLengthOfSquare() > 256 * 5)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException, "The length of the image square has to range in betwenn 0 and 1280\n");
					if (request->getQualityFactor() <= 0.0 || request->getQualityFactor() > 1.0)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,"The range of 'quality factor' has to be greater than 0 but no bigger than 1.0\n");
					if (request->getXCoordinate() < 0)
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,"The 'x' (pixel) coordinate has to be a positive integer\n");

					// TODO: check if color map is supported
					if (request->getColorMapName().empty())
						THROW_TS_EXCEPTION(tissuestack::common::TissueStackInvalidRequestException,"The 'y' (pixel) coordinate has to be a positive integer\n");

					this->_caching_strategy->extractImage(dataSet->getImageData(), dimension, request->getSliceNumber());
					const std::string response =
							tissuestack::utils::Misc::composeHttpResponse(
									"200 OK", "text/plain", "Not implemented yet!!!"
							);
					write(file_descriptor, response.c_str(), response.length());
				};

			private:
			 	std::mutex _dataset_addition_mutex;
				CachingStrategy * _caching_strategy = nullptr;
		};
	}
}

#endif	/* __IMAGE_H__ */
