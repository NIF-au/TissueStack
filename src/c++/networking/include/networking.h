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

    class RawHttpdRequest : public tissuestack::common::Request
    {
    	public:
    		RawHttpdRequest(std::string raw_content);
    		~RawHttpdRequest();
    		const std::string getRawContent() const;
    	private:
    		RawHttpdRequest(); // we forbid it, we want content
    		std::string _raw_content;
    };

    class HttpdRequest : public tissuestack::common::Request
    {
    	public:
    		HttpdRequest(std::string raw_content);
    		explicit HttpdRequest(RawHttpdRequest raw_request);
    		~HttpdRequest();
    		const std::string getHeader() const;
    		const std::string dumpHeaders() const;
    		const std::string getBody() const;
    	private:
    		HttpdRequest(); // we forbid it, we want raw content
    		std::vector<std::string> headers;
    		std::string _body;
    };

    /* TODO: let filter chain deal first with http length and basic checks
     *       then apply meta application filter: is get request ?
     *       last but not least apply action specific param checks for conversion, tiling, etc.
     *       each time 'elevate' status of request
     *       also use specific Processor variation with filters handed over
     */

    class HttpSanityFilter : public tissuestack::common::RequestFilter
    {
    	public:
    		bool applyFilter(tissuestack::common::Request& request);
    };
  }

}

#endif	/* __SERVER_H__ */
