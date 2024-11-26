#include "Server.hpp"

int main(int ac, char **av) {

	if (ac != 3)
	{
		std::cerr << "Usage: " << av[0] << " <port> <file_path>" << std::endl;
		return 1;
	}
	try
	{
		Server server(std::stoi(av[1]), av[2]);
		server.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}