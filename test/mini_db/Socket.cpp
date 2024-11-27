#include "Socket.hpp"

Socket::Socket(const int & port) : _port(port), _sockfd(-1)
{
    memset(&_servaddr, 0, sizeof(_servaddr));
    _servaddr.sin_family = AF_INET;s
    _servaddr.sin_port = htons(_port);
    _servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

Socket::~Socket() {}

void    Socket::launch(void)
{
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd < 0)
    {
        throw std::runtime_error("socket: " + std::string(strerror(errno)));
    }
    if ((bind(_sockfd, reinterpret_cast< const struct sockaddr * >(&_servaddr), sizeof(_servaddr))) < 0 )
    {
        throw std::runtime_error("bind: " + std::string(strerror(errno)));
    }    
    if ((listen(_sockfd, 128)) < 0)
    {
        throw std::runtime_error("listen: " + std::string(strerror(errno)));
    }
    if(fcntl(_sockfd, F_SETFD, O_NONBLOCK) < 0)
    {
        throw std::runtime_error("fcntl: " + std::string(strerror(errno)));
    }    
}

int     Socket::getClient(void) const
{
    return (accept(_sockfd, 0, 0));
}


int     Socket::getSocket(void) const
{
    return _sockfd;
}
