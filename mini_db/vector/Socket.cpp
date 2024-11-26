#include "Socket.hpp"

Socket::Socket(int port) : _sockfd(-1), _port(port)
{
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family =AF_INET;
    _addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    _addr.sin_port = htons(_port);
}

Socket::~Socket() 
{
    printf("socket destructor\n");
}

int    Socket::getSocket() const
{
    return _sockfd;
}

void Socket::launch()
{
    if ((_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::runtime_error("socket: " + std::string(strerror(errno)));
    }
    if ((bind(_sockfd, reinterpret_cast< const struct sockaddr * >(&_addr), sizeof(_addr))) < 0)
    {
        throw std::runtime_error("socket: " + std::string(strerror(errno)));
    }
    if (listen(_sockfd, 128) < 0)
    {
        throw std::runtime_error("socket: " + std::string(strerror(errno)));
    }
    if ((fcntl(_sockfd, F_SETFL, O_NONBLOCK)) < 0)
    {
        throw std::runtime_error("fcntl: " + std::string(strerror(errno)));
    }

    // Vérifier que le socket est bien non bloquant
    int flags = fcntl(_sockfd, F_GETFL, 0);
    if (flags & O_NONBLOCK) 
    {
        printf("Socket is non-blocking\n");
    } else 
    {
        printf("Socket is blocking\n");
    }
    
}

int    Socket::getClient()
{
    return accept(_sockfd, 0, 0);
}