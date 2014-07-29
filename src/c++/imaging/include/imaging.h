#ifndef	__IMAGE_H__
#define __IMAGE_H__

#include "logging.h"
#include "networking.h"
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

		class Dimension final
		{
			public:
				explicit Dimension(const std::string name);
				Dimension(const Dimension&) = delete;
				Dimension & operator=(const Dimension&) = delete;
				const std::string getName();
				const unsigned int getNumberOfSlices();
				const long long int getMinumum();
				const long long int getMaximum();

			private:
				const std::string 	_name;
				unsigned int 	_slices = 0;
				long long int 	_min_value = -1;
				long long int 	_max_value = -1;

		};

		class Image
		{
			public:
				explicit Image(const std::string filename);
				Image(const std::string filename, const FORMAT format);
				Image & operator=(const Image&) = delete;
				Image(const Image&) = delete;
				virtual ~Image();
				const std::string getFileName();
				virtual const bool isRaw() = 0;
				const FORMAT getFormat();
				const Dimension * const getDimension(const char dimension_letter);
				const Dimension * const getDimensionByLongName(const std::string dimension);
				const long long int getGlobalMinumum();
				const long long int getGlobalMaximum();
			protected:
				void closeFileHandle();
			private:
				const std::string		_file_name;
				FILE *			_file_handle = nullptr;
				const FORMAT _format;
				long long int 	_global_min_value = -1;
				long long int 	_global_max_value = -1;
				const std::unordered_map<char, Dimension> _dimensions;

		};

		class RawImage final : public Image
		{
			public:
				explicit RawImage(const std::string filename);
				const bool isRaw();
			private:
				const RAW_TYPE	_raw_type = RAW_TYPE::RGB_24BIT;
		};

		class NiftiImage final : public Image
		{
			public:
				~NiftiImage();
				NiftiImage(const std::string filename);
				const bool isRaw();
		};

		class MincImage final : public Image
		{
			public:
				~MincImage();
				MincImage(const std::string filename);
				const bool isRaw();
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
						const Image * image,
						const Dimension * dimension,
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
						const Image * image,
						const Dimension * dimension,
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
					this->_caching_strategy->extractImage(nullptr, nullptr, 0);
				};

			private:
				CachingStrategy * _caching_strategy = nullptr;
		};
	}
}

#endif	/* __IMAGE_H__ */
