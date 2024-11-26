#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include "Socket.hpp"
# include "DataBase.hpp"
# include <csignal>

class Socket;
class DataBase;

class Server {

	private:
		static volatile std::sig_atomic_t	_signalReceived;
		Socket 								_servSock;
		DataBase 							_db;
		int 								_max_fd;
		fd_set								_active_set, _read_set;

		static void	handleSignal(int signal);

	public:
		Server(const int & port, const std::string & file_path);
		~Server();

		void	shutdown();
		int 	run();
		void	reset_fd_set();
};

#endif