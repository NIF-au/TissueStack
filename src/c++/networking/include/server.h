#ifndef	__SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <sstream>
#include <exception>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "system.h"
#include "exceptions.h"

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
    	void stop();

      private:
    	unsigned int _port;
    	std::string _server_address;
    	int	_server_socket;
    	fd_set read_fd;

    	inline void setTimeout();
    	inline void setAddress(char * address);
    	void inline stopGracefully() const;
    	void inline shutdown() const;
    };
  }

}

#endif	/* __SERVER_H__ */
