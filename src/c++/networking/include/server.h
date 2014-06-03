#ifndef	__SERVER_H__
#define __SERVER_H__

#include "tissuestack.h"

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
	template <typename ProcessorImplementation>
    class Server final
    {
      public:
    	static const unsigned short PORT = 4242;
    	static const unsigned short MAX_REQUEST_LENGTH_IN_BYTES = 1024;
    	static const unsigned short MAX_CONNECTIONS_ALLOWED = 128;
    	static const unsigned short SHUTDOWN_TIMEOUT_IN_SECONDS = 5;

		Server & operator=(const Server&) = delete;
    	Server(const Server&) = delete;

    	Server() : Server(static_cast<unsigned int>(Server::PORT)) {};

    	explicit Server(unsigned int port): _server_socket(0), _processor(
    			tissuestack::common::RequestProcessor<ProcessorImplementation>::instance(new ProcessorImplementation()))
    	{this->_port = port;};

    	~Server() {
    		if (this->_isRunning)
    			this->stop();

    		if (_processor) delete _processor;
    	};

    	void start()
    	{
    		std::cout << "Starting Up Socket Server..." << std::endl;

    		// create a reusable server socket
    		this->_server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    		if (this->_server_socket <= 0)
    			THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to create server socket!");

    		int optVal = 1;
    		if(::setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
    			THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to change server socket options!");

    		//bind server socket to address
    		sockaddr_in server_address;
    		std::memset(&server_address, 0, sizeof(server_address));
    		server_address.sin_family = AF_INET;
    		server_address.sin_port = htons(this->_port);
    		server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    		if(::bind(this->_server_socket, (sockaddr *) &server_address, sizeof(server_address)) < 0)
    			THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to bind server socket!");

    		//listen on server socket with a pre-defined maximum of allowed connections to be queued
    		if(::listen(this->_server_socket, tissuestack::networking::Server<ProcessorImplementation>::MAX_CONNECTIONS_ALLOWED) < 0)
    			THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to listen on server socket!");

    		this->_isRunning = true;
    		std::cout << "Socket Server has been started on " << inet_ntoa(server_address.sin_addr) << ":"
    				<< this->_port << " [FD: " << this->_server_socket << "]" << std::endl;
    	};

    	/*
    	 * NOTE: We catch anything that is a controlled TissueStackApplicationException.
    	 *       Anything else is considered a more grave exception or an error even.
    	 * 		 This practice implies that the closure and its execution components MUST
    	 * 		 perform checks and throw the appropriate exceptions !!!
    	 */
    	void listen()
    	{
    		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> f =
    				[] (const tissuestack::common::ProcessingStrategy * _this)
    		 		{

    					std::cout << "Doing something ..." << std::endl;
    		 		};

   			this->_processor->process(&f);
    	};

    	void stop()
    	{
    		std::cout << "Shutting Down Socket Server..." << std::endl;
    		// stop incoming requests
    		shutdown(this->_server_socket, SHUT_RD);

    		std::cout << "Shutting Down Request Processor..." << std::endl;
			// delegate to request processor to finish off pending tasks
			this->_processor->stop();

			unsigned short shutdownTime = 0;
			while (true) // 'graceful' shutdown for up to Server::SHUTDOWN_TIMEOUT_IN_SECONDS
			{
				if (shutdownTime > Server::SHUTDOWN_TIMEOUT_IN_SECONDS)
				{
		    		std::cout << "Request Processor stopped forcefully!" << std::endl;
					break;
				} else if (!this->_processor->isRunning())
				{
		    		std::cout << "Request Processor stopped successfully!" << std::endl;
					break;
				}
				sleep(1);
				shutdownTime++;
				std::cout << "Waiting for tasks to be stopped..." << std::endl;
			}

			// close server socket
			shutdown(this->_server_socket, SHUT_WR);
			close(this->_server_socket);

			this->_isRunning = false;
    		std::cout << "Socket Server Shut Down Successfully." << std::endl;
    	};

      private:
    	unsigned int _port;
    	int	_server_socket;
    	bool _isRunning = false;
    	const tissuestack::common::RequestProcessor<ProcessorImplementation> * _processor;
    };
  }
}

#endif	/* __SERVER_H__ */
