#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include "tissuestack.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <algorithm>
#include <functional>
#include <typeinfo>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace tissuestack
{
  namespace networking
  {
	class RawHttpRequest : public tissuestack::common::Request
    {
    	public:
    		RawHttpRequest & operator=(const RawHttpRequest&) = delete;
    		RawHttpRequest(const RawHttpRequest&) = delete;
    		explicit RawHttpRequest(const std::string raw_content);
    		~RawHttpRequest();
    		const std::string getContent() const;
    		const bool isObsolete() const;
    	private:
    		const std::string _content;
    };

    class HttpRequest : public tissuestack::common::Request
    {
    	public:
    		HttpRequest & operator=(const HttpRequest&) = delete;
    		HttpRequest(const HttpRequest&) = delete;
    		explicit HttpRequest(const RawHttpRequest * const raw_request);
    		explicit HttpRequest(const RawHttpRequest * const raw_request, const bool suppress_filter);
    		~HttpRequest();
    		const std::string getParameter(std::string name) const;
    		const std::string dumpParameters() const;
    		const std::string getContent() const;
    		std::unordered_map<std::string, std::string> getParameterMap() const;
    		const bool isObsolete() const;
    	private:
    		inline void addQueryParameter(std::string key, std::string value);
    		inline void partiallyURIDecodeString(std::string& potentially_uri_encoded_string);
    		void processsQueryString();
    		inline bool skipNextCharacterCheck(int& lengthOfQueryString, int & cursor, int & nPos, std::string& key);
    		inline void subProcessQueryString(int& lengthOfQueryString, int & cursor, int & nPos, std::string& key);
    		std::unordered_map<std::string, std::string> _parameters;
    		std::string _query_string = "";
    		static std::unordered_map<std::string,std::string> MinimalURIDecodingTable;
    };

    class TissueStackImageRequest final : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE;
			TissueStackImageRequest & operator=(const TissueStackImageRequest&) = delete;
			TissueStackImageRequest(const TissueStackImageRequest&) = delete;
			explicit TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const bool isObsolete() const;
			~TissueStackImageRequest();
			const std::string getContent() const;
			const std::string getDataSetLocation() const;
			const std::string getDimensionName() const;
			const unsigned long long int getSliceNumber() const;
			const unsigned long long int getXCoordinate() const;
			const unsigned long long int getYCoordinate() const;
			const unsigned int getLengthOfSquare() const;
			const float getScaleFactor() const;
			const float getQualityFactor() const;
			const std::string getColorMapName() const;
			const std::string getOutputImageFormat() const;
		private:
			std::string _dataset_location;
			std::string _dimension_name;
			unsigned long long int _slice_number;
			unsigned long long int _x_coordinate;
			unsigned long long int _y_coordinate;
			unsigned int _length_of_square;
			float _scale_factor;
			float _quality_factor;
			std::string _color_map_name;
			std::string _output_image_format;
			unsigned long long int _request_id = 0;
			unsigned long long int _request_timestamp = 0;
    };

    class TissueStackPreTilingRequest final : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE;
    		TissueStackPreTilingRequest & operator=(const TissueStackPreTilingRequest&) = delete;
    		TissueStackPreTilingRequest(const TissueStackPreTilingRequest&) = delete;
			explicit TissueStackPreTilingRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const bool isObsolete() const;
			~TissueStackPreTilingRequest();
			const std::string getContent() const;
    };

    class TissueStackConversionRequest final : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE;
    		TissueStackConversionRequest & operator=(const TissueStackConversionRequest&) = delete;
    		TissueStackConversionRequest(const TissueStackConversionRequest&) = delete;
			explicit TissueStackConversionRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const bool isObsolete() const;
			~TissueStackConversionRequest();
			const std::string getContent() const;
    };

    class HttpRequestSanityFilter : public tissuestack::common::RequestFilter
    {
    	public:
    		HttpRequestSanityFilter & operator=(const HttpRequestSanityFilter&) = delete;
    		HttpRequestSanityFilter(const HttpRequestSanityFilter&) = delete;
    		HttpRequestSanityFilter();
			~HttpRequestSanityFilter();
    		const tissuestack::common::Request * const applyFilter(const tissuestack::common::Request * const request) const;
    };

    class TissueStackRequestFilter : public tissuestack::common::RequestFilter
    {
    	public:
			TissueStackRequestFilter & operator=(const TissueStackRequestFilter&) = delete;
			TissueStackRequestFilter(const TissueStackRequestFilter&) = delete;
			TissueStackRequestFilter();
			~TissueStackRequestFilter();
    		const tissuestack::common::Request * const applyFilter(const tissuestack::common::Request * const request) const;
    };
  }

}

#endif	/* __NETWORKING_H__ */
