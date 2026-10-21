#include "../include/Server.hpp"
#include <cstdlib>
#include <cctype>
#include <ctime>

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = av[1];
    for (size_t i = 0; i < portStr.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(portStr[i])))
        {
            std::cerr << "Error: port must be a number" << std::endl;
            return 1;
        }
    }

    int port = std::atoi(av[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: port must be between 1 and 65535" << std::endl;
        return 1;
    }

    std::string pass = av[2];
    if (pass.empty())
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }

    std::srand(static_cast<unsigned int>(std::time(NULL)));

    Server server(port, pass, -1);
    server.run();

    return 0;
}