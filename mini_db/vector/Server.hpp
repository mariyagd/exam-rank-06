#ifndef SERVER_HPP
 # define SERVER_HPP

#include "Socket.hpp"
#include "DataBase.hpp"
#include <csignal>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>

class Server
{
    private:
    static volatile std::sig_atomic_t   _signalReceived;
    Socket                              _servSock;
    DataBase                            _db;
    fd_set                              _active_set, _read_set;
    int                                 _maxfd;

    static void sigHandler(int signal);

    public:
    Server(const int & port, const std::string & filePath);
    ~Server();
    
    void    reset_fds();
    int     run();
};

#endif