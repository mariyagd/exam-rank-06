#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

class Socket
{
	private:
		int 	_sockfd;
		int		_port;

	public:
		Socket(int port): _port(port)
		{
			_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			try
			{
				if (_sockfd < 0)
					throw std::runtime_error(strerror(errno));
			}
			catch (const std::runtime_error &e)
			{
				std::cerr << e.what() << std::endl;
				exit(1);
			}
		}
		~Socket()
		{
			if (_sockfd != 1)
				close(_sockfd);
		}

		void bindAndListen()
		{
			struct sockaddr_in server_addr;
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = INADDR_ANY;	// Pour écouter sur 127.0.0.1 il faut utiliser htonl(INADDR_LOOPBACK)
			server_addr.sin_port = htons(_port);
			try
			{
				if ( bind( _sockfd, ( struct sockaddr * ) &server_addr, sizeof( server_addr ) ) < 0 )
				{
					close( _sockfd );
					throw std::runtime_error( strerror( errno ) );
				}
				if ( listen( _sockfd, 10 ) < 0 ) {
					close( _sockfd );
					throw std::runtime_error( strerror( errno ) );
				}
			}
			catch (const std::runtime_error &e)
			{
				std::cerr << e.what() << std::endl;
				exit(1);
			}
		}

		void accept()
		{
			struct sockaddr cli_address;
			memset(&cli_address, 0, sizeof(cli_address));
			socklen_t cli_len = sizeof(cli_address);

			int client_fd = ::accept(_sockfd, &cli_address, &cli_len);
			if (client_fd < 0)
			{
				close(_sockfd);
				throw std::runtime_error(strerror(errno));
			}
		}


};

class Server
{
	private:
		Socket	_listeningSocket;

	public:
		Server(int port) : _listeningSocket(port) {}
		~Server() {}

		int run() {
			_listeningSocket.bindAndListen();

			// implement the rest of the server logic here

			return 0;
		}
};

int main(int ac, char **av)
{
	Server server(atoi(av[1]));
	return server.run();
}