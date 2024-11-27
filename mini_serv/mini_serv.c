#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>

fd_set       active_set, read_set, write_set;
int          client[FD_SETSIZE - 1];
char         *msg[FD_SETSIZE - 1];
int          max_fd = 0;
int          sockfd = 0;
int          id = 0;
char         short_buffer[40];
char         long_buffer[4096];

void    error_exit(void)
{
    write(STDERR_FILENO, "Fatal error\n", 12);
    exit(1);
}

void    close_free_exit(void)
{
    for (int fd = 0; fd < max_fd + 1; fd++)
    {
        if (FD_ISSET(fd, &active_set))
        {
            close(fd);
            if (msg[fd])
            {
                free(msg[fd]);
                msg[fd] = 0;
            }
        }
    }
    error_exit();
}

void    send_to_all(int client_fd, char *message)
{
    for (int fd = 0; fd < max_fd + 1; fd++)
    {
        if (FD_ISSET(fd, &write_set) && fd != client_fd)
        {
            send(fd, message, strlen(message), 0);
        }
    }
}

void    join_message(int fd, char *long_buffer)
{
    size_t    len = 0;
    char    *new = NULL;

    if (msg[fd])
        len = strlen(msg[fd]);

    len += strlen(long_buffer) + 1;
    new = calloc(len, sizeof(*new));
    if (!new)
        close_free_exit();
    if (msg[fd])
    {
        strcat(new, msg[fd]);
        free(msg[fd]);
        msg[fd] = 0;
    }
    strcat(new, long_buffer);
    msg[fd] = new;
}

char    *str_dup(char *src)
{
    char    *copy = 0;
    
    if (!src || strlen(src) == 0)
        return 0;
    copy = calloc(strlen(src) + 1, sizeof(*copy));
    if (!copy)
        close_free_exit();
    strcpy(copy, src);
    return copy;
}

void    send_full_message(int fd)
{
    char    *message_to_send = 0;
    char    *message_to_store = 0;
    char    *store = 0;
    
    sprintf(short_buffer, "client %d: ", client[fd]);
    while (msg[fd] && (store = strstr(msg[fd], "\n")) != NULL) // if occur is NULL there is no "\n"
    {
        message_to_store = str_dup(store + 1);
        message_to_send = str_dup(msg[fd]);
        
        if (message_to_store)
            message_to_send[strlen(message_to_send) - strlen(message_to_store)] = 0;
        
        send_to_all(fd, short_buffer);
        send_to_all(fd, message_to_send);
        
        free(message_to_send);
        free(msg[fd]);
        msg[fd] = message_to_store;
    }
}

void    reset(void)
{
    memset(short_buffer, 0, sizeof(short_buffer));
    memset(long_buffer, 0, sizeof(long_buffer));
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_COPY(&active_set, &read_set);
    FD_COPY(&active_set, &write_set);
}

void    close_client(int fd)
{
    sprintf(short_buffer, "server: client %d just left\n", client[fd]);
    send_to_all(fd, short_buffer);
    close(fd);
    FD_CLR(fd, &active_set);
    if (msg[fd])
    {
        free(msg[fd]);
        msg[fd] = NULL;
    }
}

int    main(int ac, char ** av)
{
    int        ports = 0;


    if (ac != 2)
    {
        write(STDERR_FILENO, "Wrong number of arguments\n", 26);
        exit(1);
    }
    
    ports = atoi(av[1]);
    if (ports <= 1024 || ports > 65536)
        error_exit();

    // init globals
    memset(client, 0, sizeof(client));
    memset(msg, 0 , sizeof(msg));

    // start server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error_exit();

    max_fd = sockfd;
    FD_ZERO(&active_set);
    FD_SET(sockfd, &active_set);

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_port = htons(ports);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_family = AF_INET;


    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) < 0)
        close_free_exit();

    if (listen(sockfd, 128) < 0)
        close_free_exit();

    while (1)
    {
        reset();

        select(max_fd + 1, &read_set, &write_set, 0, 0);

        for (int fd = 0; fd < max_fd + 1; fd++)
        {
            if (FD_ISSET(fd, &read_set))
            {
                if (fd == sockfd)
                {
                    int client_fd = accept(fd, 0, 0);
                    if (client_fd >= 0)
                    {
                        FD_SET(client_fd, &active_set);
                        if (client_fd >  max_fd)
                            max_fd = client_fd;
                        client[client_fd] = id++;
                        
                        sprintf(short_buffer, "server: client %d just arrived\n", client[client_fd]);
                        send_to_all(client_fd, short_buffer);
                    }
                }
                else
                {
                    ssize_t    read_bytes = recv(fd, long_buffer, sizeof(long_buffer) - 1, 0);
                    if (read_bytes <= 0)
                    {
                        close_client(fd);
                    }
                    else
                    {
                        join_message(fd, long_buffer);
                        send_full_message(fd);
                    }
                }
            }
        }
    }
}



