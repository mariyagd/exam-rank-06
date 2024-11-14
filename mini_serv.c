/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdanchev <mdanchev@student.42lausanne.ch>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/13 23:47:44 by mdanchev          #+#    #+#             */
/*   Updated: 2024/11/14 09:26:33 by mdanchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// FD_SETSIZE is the maximum number of file descriptors in a set (1024)
// 1 file descriptor is used for the server, so the maximum number of clients is 1023
const int	MAX_CLIENTS = FD_SETSIZE - 1;

// set of file descriptors set for select function
// read_set are the file descriptors that are ready to send data to the server. The data is available via recv function
// write_set are the file descriptors that are ready to accept data from the server. The data is available via send function
fd_set		active_set, read_set, write_set;

// Array that stores the client's logical id for each client FD
int			client[MAX_CLIENTS];

// Array that stores the message for each client FD
char		*msg[MAX_CLIENTS];

// id is the logical id of the client
int			id = 0;

// max_fd is the highest FD that is active
int			max_fd = 0;

void	fatal_error() 
{
	write(STDERR_FILENO, "Fatal error\n", 12);
	exit(1);
}

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

void inform_others(int client_fd, char *message)
{
	for (int fd = 0; fd <= max_fd; fd++)
	{
		if (FD_ISSET(fd, &write_set) && fd != client_fd)
			if (send(fd, message, strlen(message), 0) < 0)
			{
				// subject doesn't indicate how to treat this case
				// if we close the program -> we need to close all FDs and free all messages
				close_all_fds();
				fatal_error();
			}
	}
}

void str_join(int fd, char *add)
{
	char *new = NULL;
	size_t len = 0;

	if (msg[fd])
		len = strlen(msg[fd]);

	len += strlen(add) + 1;

	new = malloc(sizeof(*new) * len);
	
	// if calloc fails return NULL
	if (!new)
	{
		// if calloc fails, free msg[fd] and return
		free(msg[fd]);
		msg[fd] = NULL;
		return ;
	}

	bzero(new, len);
	if (msg[fd])
	{
		strcat(new, msg[fd]);
		free (msg[fd]);
		msg[fd] = NULL;
	}
	strcat(new, add);
	msg[fd] = new;
	return;
}

int extract_message(int sender_fd, char **message_to_send)
{
	char *message_to_store = NULL;
	char *full_message = msg[sender_fd];
	int i = 0;
	size_t len = 0;

	if (!full_message)
		return (0);

	while(full_message[i])
	{
		if (full_message[i] == '\n')
		{
			len = strlen(full_message + i + 1) + 1;
			message_to_store = calloc(len, sizeof(*message_to_store));
			if (!message_to_store)
				// the subject doesn't indicate how to treat this case
				// if return 0 -> the full_message will be kept in msg[fd] and will be sent later
				return (0);
			strcpy(message_to_store, full_message + i + 1);
			*message_to_send = full_message;
			(*message_to_send)[i + 1] = '\0';
			msg[sender_fd] = message_to_store;
			return (1);
		}
		i++;
	}
	return (0);
}

void	send_message(int sender_fd)
{
	char * message_to_send = NULL;

	// Client's id may be 1 to 4 digits long
	// The string "client %d: " is 9 characters long
	// The maximum size of the buffer is 4 + 9 + 1 = 14
	// Let's put 16 to be sure
	char buffer[16];
	bzero(&buffer, sizeof(buffer));

	while (extract_message(sender_fd, &message_to_send))
	{
		sprintf(buffer, "client %d: ", client[sender_fd]);
		inform_others(sender_fd, buffer);
		inform_others(sender_fd, message_to_send);
		if (message_to_send)
		{
			free(message_to_send);
			message_to_send = NULL;
		}
	}
}

void	create_socket(int *server_fd)
{
	// creates an endpoint for communication and returns a FD that refers the endpoint
	// The returned FD is the lowest-numbered FD not currently open for the process
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0) 
		fatal_error();

	FD_SET(*server_fd, &active_set);
}

void	init_globals(void)
{
	//printf("init globals\n");
	FD_ZERO(&active_set);
	memset(client, -1, sizeof(client));
	memset(msg, 0, sizeof(msg));
}

/*void	reset_sets(void)
{
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);

	for (int fd = 0; fd <= max_fd; fd++) {
		if (FD_ISSET(fd, &active_set)) {
			FD_SET(fd, &read_set);
			FD_SET(fd, &write_set);
		}
	}
}*/

int main (int ac, char **av) 
{

	// printf("start\n");
	// printf("FD_SETSIZE is %d\n", FD_SETSIZE);
	// printf("SOMAXCONN is %d\n", SOMAXCONN);
	// printf("Max port: %d\n", 0xFFFF);  // 65535
	// defining here the variables
	int ports = 0, server_fd = 0;

	// short buffer is used for short informing messages
	char short_buffer[60];

	// long buffer is used to store the message received with recv
	char long_buffer[1024];

	init_globals();

	// this program accepts only one argument
	if (ac != 2) 
	{
		write(STDERR_FILENO, "Wrong number of arguments\n", 26);
		exit(1);
	}

	// Port number is a 16-bit unsigned integer, so it cannot be negative
	// The lowest TCP port number is 0 or 0x0000
	// The highest TCP port number is 65`535  or (2^16 - 1) or 0xFFFF
	// Ports < 1024 are reserved for system services
	ports = atoi(av[1]);
	//if (ports <= 0 || ports > 0xFFFF)
	if (ports <= 0 )
		fatal_error();


	create_socket(&server_fd);
	// printf("server_fd = %d\n", server_fd );
	
	// max fd is the highest actif FD, for now we have opened only the server FD
	max_fd = server_fd;

	// check in man 7 ip
	struct sockaddr_in	server_address;
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_address.sin_port = htons(ports);

	// bind the server FD to the port and the address IPv4
	if (bind(server_fd, (const struct sockaddr * )&server_address, sizeof(server_address)) < 0)
	{
		close(server_fd);
		fatal_error();
	}

	// Call listen to set the server ready for incoming connections
	// SOMAXCONN (usually 128) is the maximum number of pending connections that can be queued up before the system starts rejecting new connections
	if (listen(server_fd, SOMAXCONN) < 0)
	{
		close(server_fd);
		fatal_error();
	}

	//printf("start loop\n");
	// while the server is active
	while (1) {

		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		for (int fd = 0; fd <= max_fd; fd++) {
			if (FD_ISSET(fd, &active_set)) {
				FD_SET(fd, &read_set);
				FD_SET(fd, &write_set);
			}
		}
		// Call select to detect if a FD is ready to accept ot write
		// select() can monitor only file descriptors numbers that are less than FD_SETSIZE (1024)
		if (select(max_fd + 1, &read_set, &write_set, NULL, NULL) < 0)
		{
			close_all_fds();
			fatal_error();
		}

		// loop through all FDs
		for (int fd = 0; fd < max_fd + 1; fd++)
		{
			if (!FD_ISSET(fd, &read_set))
				continue;
			
			// if the current FD is the server, then the server is ready to accept new connections
			if (fd == server_fd)
			{
				// accept the client connection on the server
				// accept returns the FD of the new client
				int client_fd = accept(server_fd, NULL, NULL);
				
				// if accept didn't fail
				if (client_fd >= 0)
				{
					// add the new client in the active set
					FD_SET(client_fd, &active_set);

					// the new client fd may be bigger than max_fd so we need to update max_fd
					if (client_fd > max_fd)
						max_fd = client_fd;
					
					// give an id the new client
					client[client_fd] = id++;

					// printf("accepted new connection with fd %d and id %d\n", client_fd, client[client_fd]);
					
					// create a buffer with the size of the message + 1
					bzero(&short_buffer, sizeof(short_buffer));
					
					// format the message in a buffer and inform all other clients about the new arrival
					// sprintf automatically add \0 at the end of string
					sprintf(short_buffer, "server: client %d just arrived\n", client[client_fd]);
					inform_others(client_fd, short_buffer);

					// Break the "for" loop to stop looping through other file descriptors
					// In this way, the new client's fd will be set in read_set and write_set
					// and we can read and write to the new client
					break;
				}
			}
			else
			{
				//printf ("server, ready to receive\n");
				bzero(&long_buffer, sizeof(long_buffer));

				int read_bytes = recv(fd, long_buffer, sizeof(long_buffer) - 1, 0);

				// If recv fails, it return -1. The subject doesn't indicates how to treat this case.

				if (read_bytes < 0)
				{
					//printf("error recv\n");
					// subject doesn't indicate how to treat this case
					break;
				}
				// If recv returns 0 and buffer size > 0, it means that the client has left.
				if (read_bytes == 0)
				{
					// printf("client with fd %d and id %d left\n", fd, client[fd]);
				
					// create the message
					bzero(&short_buffer, sizeof(short_buffer));
					sprintf(short_buffer, "server: client %d just left\n", client[fd]);

					// inform other clients on the server
					inform_others(fd, short_buffer);

					// if the leaving client has a message that has not been send yet, free this message
					if (msg[fd])
					{
						free( msg[fd] );
						msg[fd] = NULL;
					}

					// delete the leaving client fd fron the set and close if FD
					FD_CLR(fd, &active_set);
					close(fd);
					client[fd] = -1;
					break;
				}
				// If recv returns more than 0 bytes, it means client send a message.
				// We need to store this message in msg[fd] and if the message is ready to send, we need to inform everyone about this message
				else if (read_bytes > 0)
				{
					// printf("client with fd %d and id %d send a message\n", fd, client[fd]);
					// printf("received a message %s\n", long_buffer);
					str_join(fd, long_buffer);
					send_message(fd);
				}

			}
		}
	}
}
