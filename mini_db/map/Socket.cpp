#include "Socket.hpp"

Socket::Socket(int port) : _sockfd(0), _port(port) {

	memset(&_serv_addr, 0, sizeof(_serv_addr));
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(_port);
}

Socket::~Socket() {
}

int Socket::getSocket( ) const {
	return _sockfd;
}

void	Socket::launch()
{
	if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw std::runtime_error("socket: " + std::string(strerror(errno)));
	}
	if (bind(_sockfd, reinterpret_cast< struct sockaddr * >(&_serv_addr), sizeof(_serv_addr)) < 0)
	{
		throw std::runtime_error("bind: " + std::string(strerror(errno)));
	}
	if (listen(_sockfd, 128) < 0)
	{
		throw std::runtime_error("listen: " + std::string(strerror(errno)));
	}
	printf("Server fd: %d\n", _sockfd);
}

int Socket::acceptSocket() {

	// if accept fails don't shutdown the server

	int newClient = accept( _sockfd, 0, 0);
	printf("New client fd: %d\n", newClient);
	return newClient;
}