#ifndef SERVER_HPP
 #define SERVER_HPP

#include "Socket.hpp"
#include "DataBase.hpp"
#include <sys/select.h>
#include <signal.h>
#include <iostream>
#include <unistd.h>

class Server
{
    private:
    static volatile sig_atomic_t    signalReceived;
    static void sigHandler(int signal);

    Socket      _listeningSocket;
    DataBase    _db;
    fd_set      _active_set, _read_set;
    int         _max_fd;

    public:
    Server(const int & port, const char * filePath);
    ~Server();

    void    reset_fds(void);
    int     run(void);
};

#endif