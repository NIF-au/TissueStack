#ifndef	__SERVER_H__
#define __SERVER_H__

#include "tissuestack.h"

#include <algorithm>
#include <functional>
#include <typeinfo>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>


namespace tissuestack
{
  namespace networking
  {
    class Server final
    {
      public:
    	Server & operator=(const Server&) = delete;
    	Server(const Server&) = delete;
    	Server();
    	Server(unsigned int port);
    	~Server();
    	static const unsigned short PORT = 4242;
    	static const unsigned short READ_TIMEOUT_IN_SECONDS = 5;
    	static const unsigned short MAX_REQUEST_LENGTH_IN_BYTES = 1024;
    	static const unsigned short MAX_CONNECTIONS_ALLOWED = 128;

    	unsigned int getPort() const;
    	std::string getServerAddress() const;
    	void start();
    	template <typename ProcessorImplementation>
    	void listen(const tissuestack::common::RequestProcessor<ProcessorImplementation> * const processor);
    	void stop();

      private:
    	unsigned int _port;
    	std::string _server_address;
    	int	_server_socket;


    	inline void setTimeout();
    	inline void setAddress(char * address);

    	void inline stopGracefully() const;
    	void inline shutdown() const;
    };

    class RawHttpRequest : public tissuestack::common::Request
    {
    	public:
    		RawHttpRequest & operator=(const RawHttpRequest&) = delete;
    		RawHttpRequest(const RawHttpRequest&) = delete;
    		explicit RawHttpRequest(const std::string raw_content);
    		~RawHttpRequest();
    		const std::string getContent() const;
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
    	private:
    		void addQueryParameter(std::string key, std::string value);
    		void partiallyURIDecodeString(std::string& potentially_uri_encoded_string);
    		void processsQueryString();
    		inline bool skipNextCharacterCheck(int& lengthOfQueryString, int&cursor, int&nPos, std::string& key);
    		inline void subProcessQueryString(int& lengthOfQueryString, int&cursor, int&nPos, std::string& key);
    		std::unordered_map<std::string, std::string> _parameters;
    		std::string _query_string;
    		static std::unordered_map<std::string,std::string> MinimalURIDecodingTable;
    };

    class TissueStackImageRequest final : public tissuestack::common::Request
    {
		public:
    		static const std::string SERVICE;
			TissueStackImageRequest & operator=(const TissueStackImageRequest&) = delete;
			TissueStackImageRequest(const TissueStackImageRequest&) = delete;
			explicit TissueStackImageRequest(std::unordered_map<std::string, std::string> & request_parameters);
			const bool hasExpired() const;
			~TissueStackImageRequest();
			const std::string getContent() const;
		private:
			unsigned long long int _request_id = 0;
			unsigned long long int _request_timeout = 0;
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

#endif	/* __SERVER_H__ */
