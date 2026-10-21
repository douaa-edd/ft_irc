#include "../../include/Server.hpp"

void Server::handleKICK(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.size() < 2)
    {
        sendMessage(clientFd, ":ircserv 461 KICK :Not enough parameters");
        return;
    }

    std::string chanName = msg.args[0];
    std::string reason   = msg.trailing.empty()
                           ? clients[clientFd].getNickname()
                           : msg.trailing;

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
            clients[clientFd].getNickname() + " " + chanName +
            " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(clientFd))
    {
        sendMessage(clientFd, ":ircserv 482 " +
            clients[clientFd].getNickname() + " " + chanName +
            " :You're not channel operator");
        return;
    }

    std::vector<std::string> nickList = splitByComma(msg.args[1]);

    for (size_t i = 0; i < nickList.size(); ++i)
    {
        const std::string &targetNick = nickList[i];

        if (channels.find(chanName) == channels.end())
            break;

        Channel &ch = channels[chanName];

        int targetFd = findFdByNick(targetNick);
        if (targetFd == -1)
        {
            sendMessage(clientFd, ":ircserv 401 " +
                clients[clientFd].getNickname() + " " + targetNick + " :No such nick");
            continue;
        }

        if (!ch.isMember(targetFd))
        {
            sendMessage(clientFd, ":ircserv 441 " +
                clients[clientFd].getNickname() + " " + targetNick + " " +
                chanName + " :They aren't on that channel");
            continue;
        }

        std::string kickMsg = clientPrefix(clientFd) + " KICK " + chanName +
                              " " + targetNick + " :" + reason;
        broadcastToChannel(chanName, kickMsg);

        ch.removeMember(targetFd);
        ch.removeOperator(targetFd);

        if (ch.empty())
        {
            channels.erase(chanName);
            break;
        }
    }
}