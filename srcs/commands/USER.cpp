#include "../../include/Server.hpp"

void Server::handleUSER(int clientFd, const IRCMessage &msg)
{
    if (clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 462 " +
            clients[clientFd].getNickname() + " :You may not reregister");
        return;
    }

    if (!pass.empty() && !clients[clientFd].isPassAccepted())
    {
        sendMessage(clientFd, ":ircserv 464 * :Password required");
        return;
    }

    if (msg.args.size() < 3 || (msg.args.size() < 4 && msg.trailing.empty()))
    {
        sendMessage(clientFd, ":ircserv 461 * USER :Not enough parameters");
        return;
    }

    std::string username = msg.args.empty() ? "" : msg.args[0];
    if (username.empty())
    {
        sendMessage(clientFd, ":ircserv 461 * USER :Not enough parameters");
        return;
    }

    std::string realname;
    if (!msg.trailing.empty())
        realname = msg.trailing;
    else
        realname = msg.args[3];

    clients[clientFd].setUsername(username);
    clients[clientFd].setRealname(realname);

    tryRegister(clientFd);
}