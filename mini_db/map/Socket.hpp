#ifndef SOCKET_HPP
 # define SOCKET_HPP

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/select.h>


class Socket {

private:
	int 				_sockfd;
	int 				_port;
	struct sockaddr_in _serv_addr;


public:
	Socket(int port);
	~Socket();

	int 	getSocket() const;

	void		launch();
	int 	acceptSocket();
	void	shutdown();
};

#endif