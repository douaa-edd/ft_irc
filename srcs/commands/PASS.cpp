#include "../../include/Server.hpp"

void Server::handlePASS(int clientFd, const IRCMessage &msg)
{
    if (msg.args.size() != 1)
    {
        sendMessage(clientFd, ":ircserv 461 * PASS :Not enough parameters");
        return;
    }
    
    if (clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 462 " +
            clients[clientFd].getNickname() + " :You may not reregister");
        return;
    }

    if (!clients[clientFd].getNickname().empty() ||
        !clients[clientFd].getUsername().empty())
    {
        sendMessage(clientFd, ":ircserv 462 * :You may not reregister");
        return;
    }

    if (msg.args.empty() && msg.trailing.empty())
    {
        sendMessage(clientFd, ":ircserv 461 * PASS :Not enough parameters");
        return;
    }

    std::string provided = msg.args.empty() ? msg.trailing : msg.args[0];

    if (pass == provided)
        clients[clientFd].setPassAccepted(true);
    else
    {
        clients[clientFd].setPassAccepted(false);
        sendMessage(clientFd, ":ircserv 464 * :Password incorrect");
        return;
    }

    tryRegister(clientFd);
}