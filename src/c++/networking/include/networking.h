#ifndef	__SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <sstream>
#include <exception>
#include <cstring>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"
#include "exceptions.h"
#include "utils.h"

namespace tissuestack
{
  namespace networking
  {
    class Server final
    {
      public:
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
    	void listen(tissuestack::common::RequestProcessor processor);
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
    		explicit RawHttpRequest(const std::string&& raw_content);
    		~RawHttpRequest();
    		const std::string getContent() const;
    	private:
    		std::string _content;
    };

    class HttpRequest : public tissuestack::common::Request
    {
    	public:
    		HttpRequest & operator=(const HttpRequest&) = delete;
    		HttpRequest(const HttpRequest&) = delete;
			explicit HttpRequest(RawHttpRequest & raw_request);
    		~HttpRequest();
    		const std::string getHeader() const;
    		const std::string dumpHeaders() const;
    		const std::string getContent() const;
    	private:
    		std::vector<std::string> headers;
    		std::string _content;
    };

    class HttpRequestSanityFilter : public tissuestack::common::RequestFilter
    {
    	public:
    		HttpRequestSanityFilter & operator=(const HttpRequestSanityFilter&) = delete;
    		HttpRequestSanityFilter(const HttpRequestSanityFilter&) = delete;
    		HttpRequestSanityFilter();
			~HttpRequestSanityFilter();
    		const bool applyFilter(tissuestack::common::Request & in) const;
    };

    class RequestPromoter final
    {
    	public:
    		RequestPromoter & operator=(const RequestPromoter&) = delete;
    		RequestPromoter(const RequestPromoter&) = delete;
    		RequestPromoter(); // change into singleton !!
    		virtual ~RequestPromoter();
    		const virtual tissuestack::networking::HttpRequest& promoteRequest(tissuestack::networking::RawHttpRequest & in) const;
    		const virtual tissuestack::networking::HttpRequest& promoteRequest(tissuestack::networking::HttpRequest & in) const;
    };

  }

}

#endif	/* __SERVER_H__ */
