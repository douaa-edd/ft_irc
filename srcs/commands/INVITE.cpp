#include "../../include/Server.hpp"

void Server::handleINVITE(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.size() < 2)
    {
        sendMessage(clientFd, ":ircserv 461 INVITE :Not enough parameters");
        return;
    }

    std::string targetNick = msg.args[0];
    std::string chanName   = msg.args[1];

    if (channels.find(chanName) == channels.end())
    {
        sendMessage(clientFd, ":ircserv 403 " +
            clients[clientFd].getNickname() + " " + chanName + " :No such channel");
        return;
    }

    Channel &channel = channels[chanName];

    if (!channel.isMember(clientFd))
    {
        sendMessage(clientFd, ":ircserv 442 " +
            clients[clientFd].getNickname() + " " + chanName + " :You're not on that channel");
        return;
    }

    if (channel.isInviteOnly() && !channel.isOperator(clientFd))
    {
        sendMessage(clientFd, ":ircserv 482 " +
            clients[clientFd].getNickname() + " " + chanName + " :You're not channel operator");
        return;
    }

    int targetFd = findFdByNick(targetNick);
    if (targetFd == -1)
    {
        sendMessage(clientFd, ":ircserv 401 " +
            clients[clientFd].getNickname() + " " + targetNick + " :No such nick");
        return;
    }

    if (channel.isMember(targetFd))
    {
        sendMessage(clientFd, ":ircserv 443 " +
            clients[clientFd].getNickname() + " " + targetNick + " " +
            chanName + " :is already on channel");
        return;
    }

    channel.addInvited(targetFd);

    sendMessage(clientFd,
        ":ircserv 341 " + clients[clientFd].getNickname() +
        " " + targetNick + " " + chanName);

    sendMessage(targetFd,
        clientPrefix(clientFd) + " INVITE " + targetNick + " " + chanName);
}