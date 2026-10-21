#include "../../include/Server.hpp"
#include <sstream>

void Server::handleMODE(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 461 MODE :Not enough parameters");
        return;
    }

    std::string chanName = msg.args[0];

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

    if (msg.args.size() == 1)
    {
        std::string modeStr = channel.getModeString();
        if (modeStr.empty())
            modeStr = "+";
        sendMessage(clientFd,
            ":ircserv 324 " + clients[clientFd].getNickname() +
            " " + chanName + " " + modeStr);
        return;
    }

    if (!channel.isOperator(clientFd))
    {
        sendMessage(clientFd, ":ircserv 482 " +
            clients[clientFd].getNickname() + " " + chanName + " :You're not channel operator");
        return;
    }

    std::string modeStr = msg.args[1];
    size_t paramIdx = 2;

    if (modeStr.empty())
        return;

    char sign = modeStr[0];
    if (sign != '+' && sign != '-')
    {
        sendMessage(clientFd, ":ircserv 472 " + modeStr + " :is unknown mode string");
        return;
    }


    std::string appliedModes = "";
    std::string appliedParams = "";

    for (size_t m = 1; m < modeStr.size(); m++)
    {
        char mode = modeStr[m];

        if (mode == 'i')
        {
            channel.setInviteOnly(sign == '+');
            appliedModes += mode;
        }
        else if (mode == 't')
        {
            channel.setTopicProtected(sign == '+');
            appliedModes += mode;
        }
        else if (mode == 'k')
        {
            if (sign == '+')
            {
                if (paramIdx >= msg.args.size())
                {
                    sendMessage(clientFd, ":ircserv 461 MODE :Not enough parameters");
                    return;
                }
                if (!channel.getKey().empty())
                {
                    sendMessage(clientFd, ":ircserv 467 " +
                        clients[clientFd].getNickname() + " " +
                        chanName + " :Channel key already set");
                    paramIdx++;
                    continue;
                }
                channel.setKey(msg.args[paramIdx]);
                appliedParams += " " + msg.args[paramIdx];
                paramIdx++;
            }
            else
            {
                channel.setKey("");
            }
            appliedModes += mode;
        }
        else if (mode == 'o')
        {
            if (paramIdx >= msg.args.size())
            {
                sendMessage(clientFd, ":ircserv 461 MODE :Not enough parameters");
                return;
            }
            std::string targetNick = msg.args[paramIdx++];
            int targetFd = findFdByNick(targetNick);
            if (targetFd == -1)
            {
                sendMessage(clientFd, ":ircserv 401 " +
                    clients[clientFd].getNickname() + " " + targetNick + " :No such nick");
                return;
            }
            if (!channel.isMember(targetFd))
            {
                sendMessage(clientFd, ":ircserv 441 " +
                    clients[clientFd].getNickname() + " " + targetNick + " " +
                    chanName + " :They aren't on that channel");
                return;
            }
            if (sign == '+')
                channel.addOperator(targetFd);
            else
                channel.removeOperator(targetFd);
            appliedModes += mode;
            appliedParams += " " + targetNick;
        }
        else if (mode == 'l')
        {
            if (sign == '+')
            {
                if (paramIdx >= msg.args.size())
                {
                    sendMessage(clientFd, ":ircserv 461 MODE :Not enough parameters");
                    return;
                }
                std::stringstream ss(msg.args[paramIdx]);
                int limit;
                if (!(ss >> limit) || limit <= 0)
                {
                    sendMessage(clientFd, ":ircserv 696 MODE :Invalid limit value");
                    return;
                }
                channel.setUserLimit(limit);
                appliedParams += " " + msg.args[paramIdx];
                paramIdx++;
            }
            else
            {
                channel.setUserLimit(-1);
            }
            appliedModes += mode;
        }
        else
        {
            sendMessage(clientFd, ":ircserv 472 " +
                clients[clientFd].getNickname() + " " +
                std::string(1, mode) + " :is unknown mode char to me");
        }
    }

    if (!appliedModes.empty())
    {
        std::string broadcast = clientPrefix(clientFd) + " MODE " + chanName +
                                " " + sign + appliedModes + appliedParams;
        broadcastToChannel(chanName, broadcast);
    }
}