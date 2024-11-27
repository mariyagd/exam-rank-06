#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <sys/types.h> 

fd_set		active_set, read_set, write_set;
int			client[FD_SETSIZE - 1];
char		*msg[FD_SETSIZE - 1];
int			id = 0;
int			sockfd = 0;
int			max_fd = 0;
char		short_buffer[50];
char		long_buffer[4096];


void	error_exit(void)
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	close_free_exit(void)
{
	for(int fd = 0; fd < max_fd + 1; fd++)
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

void	send_to_all(int cli, char *message)
{
	for (int fd = 0; fd < max_fd + 1; fd++)
	{
		if (FD_ISSET(fd, &write_set) && fd != cli)
		{
			send(fd, message, strlen(message), 0);
		}
	}
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
			{ 
				close_free_exit();
			}
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
	{ 
		close_free_exit();
	}
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void	init_globals(void)
{
	memset(client, 0, sizeof(client));
	memset(msg, 0, sizeof(msg));
	FD_ZERO(&active_set);
	max_fd = sockfd;
	FD_SET(sockfd, &active_set);
}

void	reset_fds(void)
{
	memset(short_buffer, 0, sizeof(short_buffer));
	memset(long_buffer, 0, sizeof(long_buffer));
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	for(int fd = 0; fd < max_fd + 1; fd++)
	{
		if (FD_ISSET(fd, &active_set))
		{
			FD_SET(fd, &read_set);
			FD_SET(fd, &write_set);
		}
	}
}

int main(int ac, char **av) {

	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) 
	{ 
		error_exit();
	}



	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
	{ 
		close_free_exit();
	} 

	if (listen(sockfd, 10) != 0) 
	{
		close_free_exit(); 
	}

	init_globals();


	while(1)
	{
		reset_fds();
		select(max_fd + 1, &read_set, &write_set, 0, 0);

		for (int fd = 0; fd < max_fd + 1; fd++)
		{
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == sockfd)
				{
					int cli = accept(sockfd, 0, 0);
					if (cli >= 0)
					{
						FD_SET(cli, &active_set);
						client[cli] = id++;
						if (cli > max_fd)
						{
							max_fd = cli;
						}

						sprintf(short_buffer, "server: client %d just arrived\n", client[cli]);
						send_to_all(cli, short_buffer);
					}
				}
				else
				{
					ssize_t read_bytes = recv(fd, long_buffer, sizeof(long_buffer) - 1, 0);
					if (read_bytes <= 0)
					{
						sprintf(short_buffer, "server: client %d just left\n", client[fd]);
						send_to_all(fd, short_buffer);
						close(fd);
						FD_CLR(fd, &active_set);
						if (msg[fd])
						{
							free(msg[fd]);
							msg[fd] = 0;
						}
					}
					else
					{
						msg[fd] = str_join(msg[fd], long_buffer);
						sprintf(short_buffer, "client %d: ", client[fd]);
						char	*msg_to_send = 0;
						while(extract_message(&msg[fd], &msg_to_send))
						{
							send_to_all(fd, short_buffer);
							send_to_all(fd, msg_to_send);
							free(msg_to_send);
						}
						
					}
				}
			}
		}
	}
}