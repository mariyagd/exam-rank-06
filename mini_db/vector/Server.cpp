#include "Server.hpp"

volatile std::sig_atomic_t   Server::_signalReceived = false;


Server::Server(const int & port, const char * filePath) : _servSock(port), _db(filePath), _maxfd(-1)
{
    FD_ZERO(&_active_set);
    FD_ZERO(&_read_set);
    std::signal(SIGINT, Server::sigHandler);
}


void Server::sigHandler(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nreceived signal\n");
        _signalReceived = true;
    }
}


Server::~Server()
{
    printf("server destructor\n");
    for (int fd = 0; fd < _maxfd + 1; fd++)
    {
        if (FD_ISSET(fd, &_active_set))
        {
            send(fd, "", 0, 0);
            printf("closing fd = %d\n", fd);
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
    }
}

void    Server::reset_fds()
{
    FD_ZERO(&_read_set);
    for (int fd = 0; fd < _maxfd + 1; fd++)
    {
        if (FD_ISSET(fd, &_active_set))
        {
            FD_SET(fd, &_read_set);
        }
    }    
}

int     Server::run()
{
    try
    {
        _db.load();
        _servSock.launch();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    _maxfd = _servSock.getSocket();
    FD_SET(_maxfd, &_active_set);

    while (!_signalReceived)
    {
        reset_fds();
        if ((select(_maxfd + 1, &_read_set, 0, 0, 0)) < 0)
        if (errno == EINTR)
        {
            printf("select stopped by a signal. Normal exit\n");
            break;
        }

        for(int fd = 0; fd < _maxfd + 1 && !_signalReceived; fd++)
        {
            if (FD_ISSET(fd, &_read_set))
            {
                if (fd == _servSock.getSocket())
                {
                    printf("new client arrived\n");
                    int cli = _servSock.getClient();
                    if (cli >= 0)
                    {
                        FD_SET(cli, &_active_set);
                        if (cli > _maxfd)
                        {
                            _maxfd = cli;
                        }
                    }
                }
                else
                {
                    char    buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t read_bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
                    if (read_bytes <= 0)
                    {
                        printf("disconnecting client\n");
                        close(fd);
                        FD_CLR(fd, &_active_set);
                    }
                    else
                    {
                        printf("treating client's request\n");
                        _db.treatRequest(fd, buffer);
                    }
                }
            }
        }
    }
    return 0;
}
