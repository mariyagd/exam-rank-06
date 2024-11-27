#ifndef SOCKET_HPP
 #define SOCKET_HPP

#include "sys/socket.h"
#include <string.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <iostream>

class Socket
{
    private:
    int                 _port;
    int                 _sockfd;
    struct sockaddr_in  _servaddr;

    public:
    Socket(const int & port);
    ~Socket();

    void    launch(void);
    int     getClient(void) const;
    int     getSocket(void) const;
};

 #endif