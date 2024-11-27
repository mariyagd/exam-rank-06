#include "Server.hpp"

volatile sig_atomic_t    Server::signalReceived = false;

void Server::sigHandler(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nsignal received\n");
        signalReceived = true;
    }
}

Server::Server(const int & port, const char * filePath) :_listeningSocket(port), _db(filePath), _max_fd(-1)
{
    FD_ZERO(&_active_set);
    FD_ZERO(&_read_set);
    signal(SIGINT, sigHandler);
}

Server::~Server()
{
    printf("server destructor\n");
    for (int fd = 0; fd < _max_fd + 1; fd++)
    {
        if (FD_ISSET(fd, &_active_set))
        {
            printf("closing fd %d\n", fd);
            close(fd);
        }
    }
}

void    Server::reset_fds(void)
{
    FD_ZERO(&_read_set);

    for (int fd = 0; fd < _max_fd + 1; fd++)
    {
        if (FD_ISSET(fd, &_active_set))
        {
            FD_SET(fd, &_read_set);
        }
    }    
}
int     Server::run(void)
{
    try
    {
        _db.load();
        _listeningSocket.launch();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    _max_fd = _listeningSocket.getSocket();
    FD_SET(_max_fd, &_active_set);

    while(!signalReceived)
    {
        reset_fds();
        select(_max_fd + 1, &_read_set, 0, 0, 0);
        if (errno == EINTR)
        {
            break;
        }

        for (int fd = 0; fd < _max_fd + 1 && !signalReceived; fd++)
        {
            if (FD_ISSET(fd, &_read_set))
            {
                if (fd == _listeningSocket.getSocket())
                {
                    int cli = _listeningSocket.getClient();
                    if (cli >= 0)
                    {
                        printf("new client with fd %d\n", cli);
                        if (cli > _max_fd)
                        {
                            _max_fd = cli;
                        }
                        FD_SET(cli, &_active_set);
                    }
                }
                else
                {
                    char buffer[1024];
                    memset(buffer, 0, sizeof(buffer));
                    ssize_t read_bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
                    if (read_bytes < 0)
                    {
                        printf("closing client %d\n", fd);
                        close(fd);
                        FD_CLR(fd, &_active_set);
                    }
                    else
                    {
                        _db.treatRequest(fd, buffer);
                    }
                }
            }
        }
    }
    return(0);
}