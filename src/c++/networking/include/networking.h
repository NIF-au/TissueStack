#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include "tissuestack.h"

#include <sys/types.h>
#include <sys/socket.h>

namespace tissuestack
{
	//forwards declarations
	namespace services
	{
		class TissueStackConversionTask;
		class TissueStackTilingTask;
	}
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
    		void dumpParametersIntoDebugLog() const;
    		const std::string getContent() const;
    		std::unordered_map<std::string, std::string> getParameterMap() const;
    		const bool isObsolete() const;
    	private:
    		inline void addQueryParameter(std::string & key, std::string value);
    		inline void partiallyURIDecodeString(std::string& potentially_uri_encoded_string);
    		void processsQueryString();
    		inline int skipNextCharacterCheck(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key);
    		inline void subProcessQueryString(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key);
    		std::unordered_map<std::string, std::string> _parameters;
    		std::string _query_string = "";
    		static std::unordered_map<std::string,std::string> MinimalURIDecodingTable;
    };

    class TissueStackImageRequest : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE1;
    		static const std::string SERVICE2;
			TissueStackImageRequest & operator=(const TissueStackImageRequest&) = delete;
			TissueStackImageRequest(const TissueStackImageRequest&) = delete;
			explicit TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters);
			TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters, bool is_preview);
			const bool isObsolete() const;
			const std::string getContent() const;
			const std::vector<std::string> getDataSetLocations() const;
			const std::string getDimensionName() const;
			const unsigned int getSliceNumber() const;
			const unsigned int getXCoordinate() const;
			const unsigned int getYCoordinate() const;
			const unsigned int getLengthOfSquare() const;
			const float getScaleFactor() const;
			const float getQualityFactor() const;
			const std::string getColorMapName() const;
			const std::string getOutputImageFormat() const;
			const unsigned short getContrastMinimum() const;
			const unsigned short getContrastMaximum() const;
			const bool isPreview() const;
			const bool hasExpired() const;
		protected:
			TissueStackImageRequest();
			void setDataSetFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setTimeStampInfoFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setDimensionFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setSliceFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setCoordinatesFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
		private:
			void setImageRequestMembersFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			bool _is_preview = false;
			std::vector<std::string> _datasets;
			std::string _dimension_name;
			unsigned int _slice_number;
			unsigned int _x_coordinate;
			unsigned int _y_coordinate;
			unsigned int _length_of_square;
			float _scale_factor;
			float _quality_factor;
			std::string _color_map_name;
			std::string _output_image_format;
			unsigned short _contrast_min;
			unsigned short _contrast_max;
			unsigned long long int _request_id = 0;
			unsigned long long int _request_timestamp = 0;
    };

    class TissueStackQueryRequest final : public TissueStackImageRequest
    {
		public:
    		static const std::string SERVICE;
    		TissueStackQueryRequest & operator=(const TissueStackQueryRequest&) = delete;
			TissueStackQueryRequest(const TissueStackQueryRequest&) = delete;
			explicit TissueStackQueryRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const std::string getContent() const;
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
		private:
			tissuestack::services::TissueStackTilingTask * _tiling = nullptr;
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
		private:
			tissuestack::services::TissueStackConversionTask * _conversion = nullptr;
    };

    class TissueStackServicesRequest final : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE;
    		TissueStackServicesRequest & operator=(const TissueStackServicesRequest&) = delete;
    		TissueStackServicesRequest(const TissueStackServicesRequest&) = delete;
			explicit TissueStackServicesRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const bool isObsolete() const;
			~TissueStackServicesRequest();
			const std::string getRequestParameter(const std::string & which, const bool convertUpperCase = false) const;
			const std::string getSubService() const;
			const std::string getContent() const;
		private:
			std::unordered_map<std::string, std::string> _request_parameters;
			std::string _subService;
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
