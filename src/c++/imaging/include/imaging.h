#ifndef	__IMAGE_H__
#define __IMAGE_H__

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
			private:
				const std::string		_file_name;
				FILE *			_file_handle = nullptr;
				const FORMAT _format;
				long long int 	_global_min_value = -1;
				long long int 	_global_max_value = -1;
				const std::unordered_map<char, Dimension> _dimensions;

		};

		class RawImage : public Image
		{
			public:
				explicit RawImage(const std::string filename);
				const bool isRaw();
			private:
				const RAW_TYPE	_raw_type = RAW_TYPE::RGB_24BIT;
		};

		class NiftiImage : public Image
		{
			public:
				NiftiImage(const std::string filename);
				const bool isRaw();
		};

		class MincImage : public Image
		{
			public:
				MincImage(const std::string filename);
				const bool isRaw();
		};
  }

}

#endif	/* __IMAGE_H__ */
