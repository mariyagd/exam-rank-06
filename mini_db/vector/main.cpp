#include "Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Wrong number of arguments" << std::endl;
        return 1;
    }
    Server server(std::atoi(av[1]), std::string(av[2]));
    return server.run();
}