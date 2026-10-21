#include "../../include/Server.hpp"

void Server::handlePART(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 461 PART :Not enough parameters");
        return;
    }

    std::string reason = msg.trailing.empty()
                         ? clients[clientFd].getNickname()
                         : msg.trailing;

    std::vector<std::string> chanList = splitByComma(msg.args[0]);

    for (size_t i = 0; i < chanList.size(); ++i)
    {
        const std::string &chanName = chanList[i];

        if (channels.find(chanName) == channels.end())
        {
            sendMessage(clientFd, ":ircserv 403 " +
                clients[clientFd].getNickname() + " " + chanName + " :No such channel");
            continue;
        }

        Channel &channel = channels[chanName];

        if (!channel.isMember(clientFd))
        {
            sendMessage(clientFd, ":ircserv 442 " +
                clients[clientFd].getNickname() + " " + chanName +
                " :You're not on that channel");
            continue;
        }

        std::string partMsg = clientPrefix(clientFd) + " PART " + chanName + " :" + reason;
        broadcastToChannel(chanName, partMsg);

        channel.removeMember(clientFd);
        channel.removeOperator(clientFd);

        if (channel.empty())
            channels.erase(chanName);
    }
}