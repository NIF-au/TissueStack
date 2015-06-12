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
#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#include "tissuestack.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

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
    		const std::string getParameter(std::string name, const bool convertToUpperCase = false) const;
    		void dumpParametersIntoDebugLog() const;
    		const std::string getContent() const;
    		const std::string getFileUploadStart() const;
    		std::unordered_map<std::string, std::string> getParameterMap() const;
    		const bool isObsolete() const;
    		const bool isFileUpload() const;
    	private:
    		inline void addQueryParameter(std::string & key, std::string value);
    		inline void partiallyURIDecodeString(std::string& potentially_uri_encoded_string);
    		void processsQueryString();
    		inline int skipNextCharacterCheck(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key);
    		inline void subProcessQueryString(int& lengthOfQueryString, int & cursor, int & nPos, std::string & key);
    		std::unordered_map<std::string, std::string> _parameters;
    		std::string _query_string = "";
    		static std::unordered_map<std::string,std::string> MinimalURIDecodingTable;
    		bool _isFileUpload = false;
    		std::string _fileUploadStart = "";
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
			const unsigned int getWidth() const;
			const unsigned int getHeight() const;
			const unsigned int getLengthOfSquare() const;
			const float getScaleFactor() const;
			const float getQualityFactor() const;
			const std::string getColorMapName() const;
			const std::string getOutputImageFormat() const;
			const unsigned short getContrastMinimum() const;
			const unsigned short getContrastMaximum() const;
			const bool showOnlyPortionOfImage() const;
			const bool isPreview() const;
			const bool hasExpired() const;
		protected:
			TissueStackImageRequest();
			void setDataSetFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setTimeStampInfoFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setDimensionFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setSliceFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			void setCoordinatesFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters, const bool is_preview = false);
		private:
			void setImageRequestMembersFromRequestParameters(const std::unordered_map<std::string, std::string> & request_parameters);
			bool _is_preview = false;
			std::vector<std::string> _datasets;
			std::string _dimension_name;
			unsigned int _slice_number = 0;
			unsigned int _x_coordinate = 0;
			unsigned int _y_coordinate = 0;
			unsigned int _length_of_square = 256;
			unsigned int _width = 0;
			unsigned int _height = 0;
			float _scale_factor = 1.0;
			float _quality_factor = 1.0;
			std::string _color_map_name = "grey";
			std::string _output_image_format = "PNG";
			unsigned short _contrast_min = 0;
			unsigned short _contrast_max = 255;
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
			const tissuestack::services::TissueStackTilingTask * getTask(const bool nullOutPointer = false);
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
			const tissuestack::services::TissueStackConversionTask * getTask(const bool nullOutPointer = false);
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
			explicit TissueStackServicesRequest(
				std::unordered_map<std::string, std::string> & request_parameters, const std::string file_upload_start = "");
			const bool isObsolete() const;
			~TissueStackServicesRequest();
			const std::string getRequestParameter(const std::string & which, const bool convertUpperCase = false) const;
			const std::string getSubService() const;
			const std::string getContent() const;
			const std::string getFileUploadStart() const;
		private:
			std::unordered_map<std::string, std::string> _request_parameters;
			std::string _subService;
			std::string _file_upload_start;
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
