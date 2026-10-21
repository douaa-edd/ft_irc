#include "../../include/Server.hpp"

void Server::handlePING(int clientFd, const IRCMessage &msg)
{
    std::string token;
    if (!msg.args.empty())
        token = msg.args[0];
    else if (!msg.trailing.empty())
        token = msg.trailing;
    else
    {
        sendMessage(clientFd, ":ircserv 409 " +
            clients[clientFd].getNickname() + " :No origin specified");
        return;
    }

    sendMessage(clientFd, ":ircserv PONG ircserv :" + token);
}