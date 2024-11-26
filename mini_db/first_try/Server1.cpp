#include "Server1.hpp"

volatile std::sig_atomic_t 	Server1::shutdown_server = 0;

Server1::Server1( int ports, const char * file_path ) : file_path( file_path)
{
	// open file here
	printf("file_path: %s\n", file_path);
	file.open(file_path);
	if (!file.is_open())
	{
		throw std::runtime_error("file open error");
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons(ports);
	sig_handler();
	launch();
}

Server1::~Server1( void )
{
	shutdown();
}

void	Server1::launch( void)
{
	printf("Server launching\n");

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		throw std::runtime_error(strerror(errno));
	}
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(server_fd);
		throw std::runtime_error(strerror(errno));
	}
	if (listen(server_fd, 128) < 0)
	{
		close(server_fd);
		throw std::runtime_error(strerror(errno));
	}
	max_fd = server_fd;
	printf("max_fd = %d\n", max_fd);
	FD_ZERO(&active_set);
	FD_SET(server_fd, &active_set);
	while (!shutdown_server)
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		read_set = active_set;
		write_set = active_set;

		if (select(max_fd + 1, &read_set, &write_set, NULL, NULL) < 0)
		{
			continue;
		}
		for (int fd = 0; fd < max_fd + 1; fd++)
		{
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == server_fd)
				{
					printf("New client\n");
					int client_fd = accept(server_fd, NULL, NULL);
					if (client_fd > FD_SETSIZE - 1)
					{
						printf("Too many clients\n");
						close(client_fd);
						continue;
					}
					if (client_fd >= 0)
					{
						if ( client_fd > max_fd )
							max_fd = client_fd;
						FD_SET( client_fd, &active_set );
					}
				}
				else
				{
					printf("Ready to receive message\n");
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
					if (bytes <= 0)
					{
						printf("Client disconnected\n");
						close(fd);
						FD_CLR(fd, &active_set);
					}
					else
					{
						printf("Received message: %s\n", buffer);
						read_message(fd, buffer);
					}
				}
			}
		}
	}
	shutdown();
}


void	Server1::shutdown( void)
{
	printf("Server shutting down\n");
	for (int fd = 0; fd <= max_fd; fd++)
	{
		if (FD_ISSET(fd, &active_set))
			close(fd);
	}
	std::map<std::string, std::string>::iterator it = db.begin();
	while (it != db.end())
	{
		file << it->first << " " << it->second << std::endl;
		it++;
	}
	file.close();
	exit(0);
}

void	Server1::handler( int sig_code)
{
	printf("Caught signal\n");
	if (sig_code == SIGINT)
		shutdown_server = 1;
}

void	Server1::sig_handler( void )
{
	struct sigaction	act;

	memset(&act, 0, sizeof(struct sigaction));
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);	// Ctrl-C
	act.sa_handler = &handler;			// handle Ctrl-C
	sigaction(SIGINT, &act, 0);
}

void Server1::send_response( int fd, const std::string & response)
{
	send(fd, response.c_str(), response.size(), 0);
}

void Server1::post( const std::string & key, const std::string & value, int fd)
{
	db.insert(std::pair<std::string, std::string>(key, value));
	send_response(fd, "0\n");
}

void Server1::get( const std::string & key, int fd)
{
	std::map<std::string, std::string>::iterator it = db.find(key);
	if (it != db.end())
	{
		send_response(fd, "0 " + it->second + "\n");
	}
	else
	{
		send_response(fd, "1\n");
	}
}

void Server1::del( const std::string & key, int fd)
{
	std::map<std::string, std::string>::iterator it = db.find(key);
	if (it != db.end())
	{
		db.erase(it);
		send_response(fd, "0\n");
	}
	else
	{
		send_response(fd, "1\n");
	}
}

void	Server1::read_message( int fd, std::string buffer)
{
	std::istringstream iss;
	iss.str(buffer);

	std::string line = "";
	while (std::getline(iss, line, '\n'))
	{
		std::istringstream iss2;
		iss2.str(line);
		while ( !iss2.eof() )
		{
			std::string command = "";
			std::string key = "";
			std::string value = "";
			iss2 >> command >> key;
			printf("command: %s\n", command.c_str());
			printf("key: %s\n", key.c_str());
			if (command == "POST")
			{
				iss2 >> value;
				printf("value: %s\n", value.c_str());
				post(key, value, fd);
			}
			else if (command == "GET")
			{
				get(key, fd);
			}
			else if (command == "DEL")
			{
				del(key, fd);
			}
			else
			{
				send_response(fd, "2\n");
			}
		}
	}
}