#include "Server.hpp"
#include <cstdlib>

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Wrong number of arguments" << std::endl;
        return 1;
    }
    Server server(std::atoi(av[1]), av[2]);
    return server.run();
}