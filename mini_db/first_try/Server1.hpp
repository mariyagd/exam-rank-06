#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sstream>
#include <map>
#include <fstream>

class Server1 {

	private:
		int 										server_fd, max_fd;
		fd_set 										active_set, read_set, write_set;
		struct sockaddr_in							server_addr;
		static volatile std::sig_atomic_t 			shutdown_server;
		std::string 								file_path;
		std::ofstream 								file;
		std::vector<int>							clients;
		std::map<std::string , std::string>			db;


		static void		handler(int sig_code);
		void			sig_handler(void);

	public:
		Server1( int ports, const char * file_path );
		~Server1( void );

		void			launch(void);
		void			shutdown(void);
		void			read_message(int fd, std::string buffer);
		void 			post(const std::string & key, const std::string & value, int fd);
		void			get(const std::string & key, int fd);
		void			del(const std::string & key, int fd);
		void			send_response(int fd, const std::string & response);
};