#ifndef SOCKET_HPP
 # define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdexcept>
#include <stdio.h>
#include <sys/errno.h>
#include <string>
#include <fcntl.h>

class Socket 
{
    private:
    int                 _sockfd;
    int                 _port;
    struct sockaddr_in  _addr;

    public:
    Socket(int port);
    ~Socket();
    
    int     getSocket() const;
    void    launch();
    int     getClient();
};

#endif