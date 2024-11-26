#include "Server.hpp"

volatile std::sig_atomic_t Server::_signalReceived = false;

Server::Server(const int & port, const std::string & file_path) : _servSock(port), _db(file_path) {
	std::signal(SIGINT, Server::handleSignal);
	FD_ZERO(&_active_set);
}

Server::~Server() {
	shutdown();
}

void Server::shutdown() {

	printf("Shutting down server\n");
	printf("Closing fd: ");
	for (int fd = 0; fd < _max_fd + 1; fd++)
	{
		if ( FD_ISSET(fd, &_active_set))
		{
			printf( "%d ", fd );
			close( fd );
		}
	}
	printf("\n");
}

void Server::handleSignal(int signal) {

	if (signal == SIGINT)
	{
		printf("\nSignal received\n");
		_signalReceived = true;
	}
}

void Server::reset_fd_set() {

	FD_ZERO(&_read_set);
	for (int fd = 0; fd < _max_fd + 1; fd++)
	{
		if ( FD_ISSET(fd, &_active_set))
		{
			FD_SET(fd, &_read_set);
		}
	}
}

int Server::run() {

	try
	{
		_db.load();
		_servSock.launch();
	}
	catch (std::exception &e)
	{
		throw;
	}
	_max_fd = _servSock.getSocket();
	FD_SET(_max_fd, &_active_set);

	printf("Server running\n");
	while ( _signalReceived == false )
	{
		reset_fd_set();
		select(_max_fd + 1, &_read_set, 0, 0, 0);

		for (int fd = 0; fd < _max_fd + 1 && _signalReceived == false; fd++)
		{
			if ( FD_ISSET(fd, &_read_set))
			{
				if (fd == _servSock.getSocket())
				{
					int newClient = _servSock.acceptSocket();
					if (newClient >= 0)
					{
						FD_SET( newClient, &_active_set );
						if ( newClient > _max_fd )
							_max_fd = newClient;
					}
				}
				else
				{
					printf("ready to read\n");
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					ssize_t readBytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
					if (readBytes <= 0)
					{
						printf("Client %d disconnected\n", fd);
						close(fd);
						FD_CLR(fd, &_active_set);
					}
					else
					{
						printf("Received: %s", buffer);
						_db.treatRequest(fd, buffer);
					}
				}
			}
		}
	}
	return 0;
}

