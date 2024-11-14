#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/ip.h>

fd_set		active_set, read_set, write_set;
const int	MAX_CLIENTS = FD_SETSIZE - 1;
int			client[MAX_CLIENTS];
char		*msg[MAX_CLIENTS];
int			id = 0;
int			max_fd = 0;


void close_all_fds(void)
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
}

void	fatal_error(void)
{
	write(STDERR_FILENO, "Fatal error\n", 12);
	exit(1);
}

void	inform_others(int client_fd, char *message)
{
	for (int fd = 0; fd < max_fd + 1; fd++ ) 
	{
		if (FD_ISSET(fd, &write_set) && fd != client_fd) 
		{
			send(fd, message, strlen(message), 0);
		}
	}
}

void	str_join(int fd, char *add)
{
	char 	*new = NULL;
	size_t	len = 0;

	if (msg[fd])
		len = strlen(msg[fd]);
	
	len += strlen(add) + 1;
	new = calloc(len, sizeof(*new));

	if (!new)
	{
		free(msg[fd]);
		msg[fd] = NULL;
		return;
	}

	if (msg[fd])
	{
		strcat(new, msg[fd]);
		free(msg[fd]);
		msg[fd] = NULL;
	}
	strcat(new, add);
	msg[fd] = new;
	return;
}

int	extract_message(char **message_to_send, int fd)
{
	char	*message_to_store = NULL;
	char	*full_message = msg[fd];
	size_t		len = 0;
	int			i = 0;

	if (!full_message)
		return (0);

	while (full_message[i])
	{
		if (full_message[i] == '\n')
		{
			len = strlen(full_message + i + 1) + 1;
			message_to_store = calloc(len, sizeof(*message_to_store));
			if (!message_to_store)
				return (0);
			strcpy(message_to_store, full_message + i + 1);
			*message_to_send = full_message;
			(*message_to_send)[i + 1] = '\0';
			msg[fd] = message_to_store;
			return (1);
		}
		i++;
	}
	return (0);
}

void	send_message(int client_fd)
{
	char	*message_to_send = NULL;
	char	buffer[60];

	while (extract_message(&message_to_send, client_fd))
	{
		bzero(&buffer, sizeof(buffer));
		sprintf(buffer, "client %d: ", client[client_fd]);
		inform_others(client_fd, buffer);
		inform_others(client_fd, message_to_send);
		if (message_to_send)
		{
			free(message_to_send);
			message_to_send = NULL;
		}
	}

}

void	create_server_socket(int *server_fd)
{
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (*server_fd < 0)
		fatal_error();

	FD_SET(*server_fd, &active_set);
}

void	init_globals(void)
{
	FD_ZERO(&active_set);
	memset(client, -1, sizeof(client));
	memset(msg, 0, sizeof(msg));
}

void	reset_fds(void)
{
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	for (int fd = 0; fd < max_fd + 1; fd++) 
	{
		if (FD_ISSET(fd, &active_set))
		{
			FD_SET(fd, &read_set);
			FD_SET(fd, &write_set);
		}
	}
}

int	main(int ac, char **av)
{
	int		ports = 0, server_fd = 0;
	char	short_buffer[60];
	char	long_buffer[1024];

	if (ac != 2)
	{
		write(STDERR_FILENO, "Wrong number of arguments\n", 26);
		exit(1);
	}
	
	ports = atoi(av[1]);
	if (ports <= 0 || ports > 0xFFFF)
		fatal_error();

	init_globals();
	create_server_socket(&server_fd);
	max_fd = server_fd;

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_address.sin_port = htons(ports);
	
	if (bind(server_fd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		close(server_fd);
		fatal_error();
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		close(server_fd);
		fatal_error();
	}

	while (1)
	{
		reset_fds();

		if (select(max_fd + 1, &read_set, &write_set, NULL, NULL) < 0)
		{
			close_all_fds();
			fatal_error();
		}

		for (int fd = 0; fd < max_fd + 1; fd++)
		{
			if ( !FD_ISSET( fd, &read_set ) )
				continue;

			if ( fd == server_fd )
			{
				int client_fd = accept( server_fd, NULL, NULL );

				if ( client_fd >= 0 )
				{
					FD_SET( client_fd, &active_set );
					if ( client_fd > max_fd )
						max_fd = client_fd;

					client[client_fd] = id++;

					bzero( &short_buffer, sizeof( short_buffer ) );
					sprintf( short_buffer, "server: client %d just arrived\n", client[client_fd] );
					inform_others( client_fd, short_buffer );
					break;
				}
			}
			else
			{
				bzero( &long_buffer, sizeof( long_buffer ) );

				int read_bytes = ( int ) recv( fd, long_buffer, sizeof( long_buffer ) - 1, 0 );

				if ( read_bytes == 0 )
				{
					bzero( &short_buffer, sizeof( short_buffer ) );
					sprintf( short_buffer, "server: client %d just left\n", client[fd] );
					inform_others( fd, short_buffer );

					FD_CLR( fd, &active_set );
					close( fd );
					if ( msg[fd] )
					{
						free( msg[fd] );
						msg[fd] = NULL;
					}
					client[fd] = -1;
					break;
				}
				else if ( read_bytes > 0 )
				{
					str_join( fd, long_buffer );
					send_message( fd );
				}
			}
		}
	}

}
