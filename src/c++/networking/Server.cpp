#include "server.h"

tissuestack::networking::Server::Server() :
	tissuestack::networking::Server::Server(static_cast<unsigned int>(tissuestack::networking::Server::PORT)) {}

tissuestack::networking::Server::Server(unsigned int port) : _server_socket(0)
{
	this->_port = port;
}

tissuestack::networking::Server::~Server()
{
	std::cout << "Cleaning Up Socket Resources..." << std::endl;
	close(this->_server_socket);
}

void tissuestack::networking::Server::start()
{
	std::cout << "Starting Up Socket Server..." << std::endl;

	/* create a reusable server socket */
	this->_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_server_socket <= 0)
		throw tissuestack::common::TissueStackException(std::string("Failed to create server socket!"));
	int optVal = 1;
	if(setsockopt(this->_server_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) != 0)
		throw tissuestack::common::TissueStackException(std::string("Failed to change server socket options!"));

	/* bind server socket to address */
	sockaddr_in server_address;
	std::memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(this->_port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	this->setAddress(inet_ntoa(server_address.sin_addr));

	if(bind(this->_server_socket, (sockaddr *) &server_address, sizeof(server_address)) < 0)
		throw tissuestack::common::TissueStackException(
				std::string("Failed to bind server socket to address: ").append(this->_server_address));

	/* listen on server socket with a pre-defined maximum of allowed connections to be queued */
	if(listen(this->_server_socket, tissuestack::networking::Server::MAX_CONNECTIONS_ALLOWED) < 0)
		throw tissuestack::common::TissueStackException(
				std::string("Failed to listen on server socket!"));

	struct sockaddr_in client_address;
	socklen_t sin_length = sizeof(client_address);
	int client_socket=0;

	if((client_socket = accept(this->_server_socket, (sockaddr *) &client_address, &sin_length)) <= 0)
		throw tissuestack::common::TissueStackException(
					std::string("Failed to accept connections on server socket!"));

	// TODO: to be continued

	std::cout << "Client connected from: " << inet_ntoa(client_address.sin_addr) << std::endl ;

	std::cout << "Socket Server Listening On " << this->_server_address << ":"
			<< this->_port << " [FD: " << this->_server_socket << "]" << std::endl;
	std::cout << "Request Size Limit: " << tissuestack::networking::Server::MAX_REQUEST_LENGTH_IN_BYTES << std::endl;
	std::cout << "Read Timeout (s): " << tissuestack::networking::Server::READ_TIMEOUT_IN_SECONDS << std::endl;
}

void tissuestack::networking::Server::stop()
{
	std::cout << "Shutting Down Socket Server..." << std::endl;
	std::cout << "Socket Server Shut Down Successfully." << std::endl;
}

inline void tissuestack::networking::Server::setTimeout()
{

	timeval timeout;
	timeout.tv_sec = static_cast<decltype(timeout.tv_sec)>(tissuestack::networking::Server::READ_TIMEOUT_IN_SECONDS);
	timeout.tv_usec = static_cast<decltype(timeout.tv_sec)>(0);

	//return timout;
}

inline void tissuestack::networking::Server::setAddress(char * address)
{
	std::ostringstream in;
	in << address;
	this->_server_address = in.str();
}
