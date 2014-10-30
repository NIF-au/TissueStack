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
#ifndef	__SERVER_H__
#define __SERVER_H__

#include "networking.h"
#include "imaging.h"
#include "execution.h"
#include "database.h"
#include "services.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>

#include <unistd.h>
#include <arpa/inet.h>

namespace tissuestack
{
  namespace networking
  {
  	  static const unsigned short MAX_CONNECTIONS = 1024;
  	  template <typename ProcessorImplementation> class Server;

	  template <typename ProcessorImplementation>
  	  class ServerSocketSelector final
  	  {
  	  	  private:
    		const tissuestack::networking::Server<ProcessorImplementation> * _server;
    		tissuestack::execution::TissueStackOnlineExecutor * _executor = nullptr;

  		public:
    		ServerSocketSelector(const tissuestack::networking::Server<ProcessorImplementation> * server) :
    			_server(server) {
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

    		void dispatchRequest(int request_descriptor, const std::string request_data)
    		{
    			const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * f = new
    					std::function<void (const tissuestack::common::ProcessingStrategy * _this)>(
    				  [this, request_data, request_descriptor] (const tissuestack::common::ProcessingStrategy * _this)
    				  {
    					try
    					{
    						this->_executor->execute(_this, request_data, request_descriptor);
    					}  catch (std::exception& bad)
    					{
    						// close connection and log error
    						close(request_descriptor);
    						tissuestack::logging::TissueStackLogger::instance()->error("Something bad happened: %s\n", bad.what());
    					}
    				  });
    			this->_server->_processor->process(f);
      		};

  			void startEventLoop()
  			{
				// create the epoll 'controller'
				int epollController = epoll_create(1);
				if(epollController == -1)
  					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException,
  						"Failed to start EPOLLing!");

				struct epoll_event  epollEvent;
				epollEvent.data.fd = this->_server->getServerSocket(); // our server socket
				epollEvent.events = EPOLLIN; // for READS only

				if (epoll_ctl (epollController, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent) == -1)
  					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException,
  						"Failed to start EPOLLing!");

				struct epoll_event clientEvents[tissuestack::networking::MAX_CONNECTIONS];

				// loop for events until we stop the server
				while(this->_server->isRunning() && !this->_server->isStopping())
				{
					int numEvents =
						epoll_wait(
							epollController,
							clientEvents,
							tissuestack::networking::MAX_CONNECTIONS,
							-1);

					// loop over event client triggered events ...
					for (int i = 0; i < numEvents; i++)
					{
						// something went wrong when epolling ...
						if ((clientEvents[i].events & EPOLLERR) ||
								(clientEvents[i].events & EPOLLHUP) ||
								(!(clientEvents[i].events & EPOLLIN)))
							continue;

						// we have a new client connecting
						if (clientEvents[i].data.fd == this->_server->getServerSocket())
						{
							struct sockaddr_in new_client;
							unsigned int addrlen = sizeof(new_client);

							// accept new client
							int new_fd = accept(this->_server->getServerSocket(), (struct sockaddr *) &new_client, &addrlen);
							if (!tissuestack::utils::System::makeSocketNonBlocking(new_fd))
								THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to make server socket non-blocking!");

							// check accept status
							if (new_fd  == -1 )  // NOK
							{
								if (!this->_server->isStopping())
									tissuestack::logging::TissueStackLogger::instance()->error("Failed to accept client connection!\n");
							} else
							{
								struct epoll_event ev;
								ev.data.fd = new_fd;
								ev.events = EPOLLIN; //  read
								if (epoll_ctl(epollController, EPOLL_CTL_ADD, new_fd, &ev) == -1)
									THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException,
										"Failed to add client to epoll list!");
								continue;
							}
						}	else // else: we have data to be read from one of the connecting clients
						{
							// do an initial read with a pre-defined buffer size
							int fd = clientEvents[i].data.fd;
							char data_buffer[tissuestack::common::SOCKET_READ_BUFFER_SIZE];
							ssize_t bytesReceived = recv(fd, data_buffer, sizeof(data_buffer), 0);
							std::string raw_content = "";

							if (bytesReceived > 0)
							{

								raw_content = std::string(data_buffer, bytesReceived);
								// we need to explicitly remove the file uploads from sending more events ...
								if (raw_content.find("POST") == 0 &&
									raw_content.find("service=services") != std::string::npos &&
									raw_content.find("sub_service=admin") != std::string::npos &&
									raw_content.find("action=upload") != std::string::npos)
									epoll_ctl (epollController, EPOLL_CTL_DEL, fd, NULL);
								else
								{
									// read till we have EAGAIN
									std::ostringstream dataInputStream;
									dataInputStream << raw_content;

									while ((bytesReceived = recv(fd, data_buffer, sizeof(data_buffer), 0)) > 0)
									{
										raw_content = std::string(data_buffer, bytesReceived);
										dataInputStream << raw_content;
									}
									raw_content = dataInputStream.str();
								}
								this->dispatchRequest(fd, raw_content);
							}
						}
					} // end event loop
				} // end polling loop
			close(epollController); // close polling controller
  		};
  	};

  	template <typename ProcessorImplementation>
    class Server final
    {
 		friend class tissuestack::networking::ServerSocketSelector<ProcessorImplementation>;
    	public:
			static const unsigned short PORT = 4242;
			static const unsigned short SHUTDOWN_TIMEOUT_IN_SECONDS = 10;

			Server & operator=(const Server&) = delete;
			Server(const Server&) = delete;

			explicit Server(unsigned int port=4242): _server_socket(0), _processor(
					tissuestack::common::RequestProcessor<ProcessorImplementation>::instance(new ProcessorImplementation()))
			{this->_port = port;};

			~Server() {
				if (this->_isRunning)
					this->stop();

				if (this->_processor) delete this->_processor;
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
				tissuestack::logging::TissueStackLogger::instance()->info("Starting Up Socket Server...\n");

				// create a reusable server socket
				this->_server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
				if (this->_server_socket <= 0)
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to create server socket!");

				int optVal = 1;
				if(setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to change server socket options!");

				if (!tissuestack::utils::System::makeSocketNonBlocking(this->_server_socket))
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to make server socket non-blocking!");

				//bind server socket to address
				sockaddr_in server_address;
				std::memset(&server_address, 0, sizeof(server_address));
				server_address.sin_family = AF_INET;
				server_address.sin_port = htons(this->_port);
				server_address.sin_addr.s_addr = htonl(INADDR_ANY);

				if(::bind(this->_server_socket, (sockaddr *) &server_address, sizeof(server_address)) < 0)
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to bind server socket!");

				//listen on server socket with a pre-defined maximum of allowed connections to be queued
				if(::listen(this->_server_socket, tissuestack::networking::MAX_CONNECTIONS) < 0)
					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Failed to listen on server socket!");

				this->_isRunning = true;
				tissuestack::logging::TissueStackLogger::instance()->info("Socket Server has been started on %s:%u [FD: %i]\n",
						inet_ntoa(server_address.sin_addr), this->_port, this->_server_socket);
			};

			void listen()
			{
				tissuestack::logging::TissueStackLogger::instance()->info("Socket Server is now ready to accept requests...\n");

				this->_processor->init();
				// delegate to the selector class
				tissuestack::networking::ServerSocketSelector<ProcessorImplementation> SocketSelector(this);
				SocketSelector.startEventLoop();
			};

			void stop()
			{
				tissuestack::logging::TissueStackLogger::instance()->info("Shutting Down Socket Server...\n");
				// stop incoming requests
				this->_stopRaised = true;
				shutdown(this->_server_socket, SHUT_RD);

				unsigned short shutdownTime = 0;
				while (true) // 'graceful' shutdown for up to Server::SHUTDOWN_TIMEOUT_IN_SECONDS
				{
					tissuestack::logging::TissueStackLogger::instance()->info("Waiting for tasks to be stopped...\n");
					// delegate to request processor to finish off pending tasks
					this->_processor->stop();

					if (shutdownTime > Server::SHUTDOWN_TIMEOUT_IN_SECONDS)
					{
						tissuestack::logging::TissueStackLogger::instance()->info("Request Processor will be stopped forcefully!\n");
						break;
					} else if (!this->_processor->isRunning())
					{
						tissuestack::logging::TissueStackLogger::instance()->info("Request Processor stopped successfully!\n");
						break;
					}
					sleep(1);
					shutdownTime++;
				}

				// close server socket
				shutdown(this->_server_socket, SHUT_WR);
				close(this->_server_socket);

				this->_isRunning = false;

				tissuestack::logging::TissueStackLogger::instance()->info("Socket Server Shut Down Successfully.\n");
				try
				{
					tissuestack::logging::TissueStackLogger::instance()->purgeInstance();
				} catch (...)
				{
					// can be safely ignored
				}

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
