#ifndef	__SERVER_H__
#define __SERVER_H__

#include "tissuestack.h"
#include "networking.h"
#include "execution.h"

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
  	  template <typename ProcessorImplementation> class Server;

	  template <typename ProcessorImplementation>
  	  class ServerSocketSelector final
  	  {
  	  	  private:
    		const tissuestack::networking::Server<ProcessorImplementation> * _server;
    		tissuestack::execution::TissueStackOnlineExecutor * _executor = nullptr;
    		fd_set _master_descriptors;
    		fd_set _tmp_descriptors;

  		public:
        	static const unsigned short MAX_REQUEST_LENGTH_IN_BYTES = 1024;

    		ServerSocketSelector(const tissuestack::networking::Server<ProcessorImplementation> * server) : _server(server) {
  				if (server == nullptr || server->isStopping() || !server->isRunning())
  					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException,
  							"ServerSocket was either handed a null instance of a server object or the server is stopping/not running anyway!");

  				this->_executor = tissuestack::execution::TissueStackOnlineExecutor::instance();
  			};

    		~ServerSocketSelector()
    		{
    			if (this->_executor)
    				delete this->_executor;
    		};

    		void closeConnectionAndRemoveDescriptorFromList(int descriptor)
    		{
    			if (descriptor <= 0) return;

				// close connection and remove from descriptor list
				close(descriptor);
				FD_CLR(descriptor, &this->_master_descriptors);
    		};

    		void dispatchRequest(int request_descriptor, const char * request_data)
    		{
    			const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> f =
    				  [this, &request_data, &request_descriptor] (const tissuestack::common::ProcessingStrategy * _this)
    				  {
						this->_executor->execute(std::string(request_data), request_descriptor);
						this->closeConnectionAndRemoveDescriptorFromList(request_descriptor);
    				  };
    			this->_server->_processor->process(&f);
      		};

  			void startSelectLoop()
  			{
  				// clear the descriptors
				FD_ZERO(&this->_master_descriptors);
				FD_ZERO(&this->_tmp_descriptors);

				// maximum file descriptor
				int max_fd;

				// add the server socket to the master set and set it as the maximum descriptor
				FD_SET(this->_server->getServerSocket(), &this->_master_descriptors);
				max_fd = this->_server->getServerSocket();

				// loop until we received stop
				while (this->_server->isRunning() && !this->_server->isStopping()) {
					// copy over descriptors
  					this->_tmp_descriptors = this->_master_descriptors;

  					if (select(max_fd + 1, &this->_tmp_descriptors, NULL, NULL, NULL) == -1)
  					{
  						if (this->_server->isStopping()) break;
  						std::cerr << "ServerSocket select returned -1" << std::endl;
  					}

					// check existing descriptor list
					for (int i = 0; i <= max_fd; i++) {
						if (FD_ISSET(i, &this->_tmp_descriptors)) { // something happened
							if (i == this->_server->getServerSocket())  // we have a new client connecting
							{
				  				struct sockaddr_in new_client;
				  				unsigned int addrlen = sizeof(new_client);
				  				// accept new client
								int new_fd = accept(this->_server->getServerSocket(), (struct sockaddr *) &new_client, &addrlen);

								// check accept status
								if (new_fd  == -1) { // NOK
									std::cerr << "Failed to accept client connection!" << std::endl;
								} else
								{
									FD_SET(new_fd, &this->_master_descriptors); // add to master descriptor list
									if (new_fd > max_fd) // keep track of the maximum
  										max_fd = new_fd;

									std::cout << "New connection from " << inet_ntoa(new_client.sin_addr)
											<< " (fd: " << new_fd << ")" << std::endl;
  								}
							} else // we are ready to receive from an existing client connection
							{
								char data_buffer[MAX_REQUEST_LENGTH_IN_BYTES+1];
								int bytesReceived = recv(i, data_buffer, sizeof(data_buffer), 0);
								if (bytesReceived <= 0 || bytesReceived > MAX_REQUEST_LENGTH_IN_BYTES) { // NOK case
									// client close
									if (bytesReceived == 0)
										std::cerr << "Client closed connection!" << std::endl;
									else if (bytesReceived < 0)
										std::cerr << "Data Receive error!" << std::endl;
									else if (bytesReceived > MAX_REQUEST_LENGTH_IN_BYTES) // for now we have a limit
										std::cerr << "Exceeded Request Size Allowed: " << MAX_REQUEST_LENGTH_IN_BYTES << " !!" << std::endl;

									this->closeConnectionAndRemoveDescriptorFromList(i);
								}
								else // OK case
								{
									this->dispatchRequest(i, const_cast<const char *>(data_buffer));
								}
							}
						}
					}
				}
  			};
  	};

  	template <typename ProcessorImplementation>
    class Server final
    {
 		friend class tissuestack::networking::ServerSocketSelector<ProcessorImplementation>;
    	public:
			static const unsigned short PORT = 4242;
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

			int getServerSocket() const
			{
				return this->_server_socket;
			}

			bool isRunning() const
			{
				return this->_isRunning;
			}

			bool isStopping() const
			{
				return this->_stopRaised;
			}

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

			void listen()
			{
				std::cout << "Socket Server is now ready to accept requests..." << std::endl;

				this->_processor->init();
				// delegate to the selector class
				tissuestack::networking::ServerSocketSelector<ProcessorImplementation> SocketSelector(this);
				SocketSelector.startSelectLoop();
			};

			void stop()
			{
				std::cout << "Shutting Down Socket Server..." << std::endl;
				// stop incoming requests
				this->_stopRaised = true;
				shutdown(this->_server_socket, SHUT_RD);

				unsigned short shutdownTime = 0;
				while (true) // 'graceful' shutdown for up to Server::SHUTDOWN_TIMEOUT_IN_SECONDS
				{
					std::cout << "Waiting for tasks to be stopped..." << std::endl;
					// delegate to request processor to finish off pending tasks
					this->_processor->stop();

					if (shutdownTime > Server::SHUTDOWN_TIMEOUT_IN_SECONDS)
					{
						std::cout << "Request Processor will be stopped forcefully!" << std::endl;
						break;
					} else if (!this->_processor->isRunning())
					{
						std::cout << "Request Processor stopped successfully!" << std::endl;
						break;
					}
					std::this_thread::sleep_for(std::chrono::seconds(1));
					shutdownTime++;
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
			bool _stopRaised = false;
			const tissuestack::common::RequestProcessor<ProcessorImplementation> * _processor;
    };
  }
}

#endif	/* __SERVER_H__ */
