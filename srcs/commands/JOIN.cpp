#include "../../include/Server.hpp"

static void sendNamesReply(Server &srv, int clientFd,
                            Channel &channel,
                            const std::map<int, Client> &clients)
{
    std::string nick  = clients.at(clientFd).getNickname();
    std::string names = "";

    const std::vector<int> &members = channel.getMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        std::map<int, Client>::const_iterator it = clients.find(members[i]);
        if (it == clients.end())
            continue;
        if (channel.isOperator(members[i]))
            names += "@";
        names += it->second.getNickname();
        if (i + 1 < members.size())
            names += " ";
    }

    srv.sendMessage(clientFd,
        ":ircserv 353 " + nick + " = " + channel.getName() + " :" + names);

    srv.sendMessage(clientFd,
        ":ircserv 366 " + nick + " " + channel.getName() + " :End of NAMES list");
}

void Server::handleJOIN(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 461 JOIN :Not enough parameters");
        return;
    }

    std::vector<std::string> chanList = splitByComma(msg.args[0]);
    std::vector<std::string> keyList;
    if (msg.args.size() >= 2)
        keyList = splitByComma(msg.args[1]);

    for (size_t i = 0; i < chanList.size(); ++i)
    {
        if (clients.find(clientFd) == clients.end())
            break;

        const std::string &chanName = chanList[i];
        std::string chanKey = (i < keyList.size()) ? keyList[i] : "";

        if (chanName.empty() || chanName[0] != '#')
        {
            sendMessage(clientFd, ":ircserv 476 " + chanName + " :Bad channel name");
            continue;
        }

        if (channels.find(chanName) == channels.end())
        {
            channels[chanName] = Channel(chanName);
            Channel &ch = channels[chanName];
            ch.addMember(clientFd);
            ch.addOperator(clientFd);

            if (!chanKey.empty())
                ch.setKey(chanKey);

            std::string joinMsg = clientPrefix(clientFd) + " JOIN " + chanName;
            sendMessage(clientFd, joinMsg);

            sendMessage(clientFd,
                ":ircserv 331 " + clients[clientFd].getNickname() +
                " " + chanName + " :No topic is set");

            sendNamesReply(*this, clientFd, ch, clients);
            continue;
        }

        Channel &channel = channels[chanName];

        if (channel.isMember(clientFd))
            continue;

        if (channel.isInviteOnly() && !channel.isInvited(clientFd))
        {
            sendMessage(clientFd, ":ircserv 473 " +
                clients[clientFd].getNickname() + " " +
                chanName + " :Cannot join channel (+i)");
            continue;
        }

        if (!channel.getKey().empty())
        {
            if (chanKey != channel.getKey())
            {
                sendMessage(clientFd, ":ircserv 475 " +
                    clients[clientFd].getNickname() + " " +
                    chanName + " :Cannot join channel (+k)");
                continue;
            }
        }

        if (channel.getUserLimit() != -1 &&
            channel.getMemberCount() >= channel.getUserLimit())
        {
            sendMessage(clientFd, ":ircserv 471 " +
                clients[clientFd].getNickname() + " " +
                chanName + " :Cannot join channel (+l)");
            continue;
        }

        channel.addMember(clientFd);

        if (channel.isInvited(clientFd))
            channel.removeInvited(clientFd);

        std::string joinMsg = clientPrefix(clientFd) + " JOIN " + chanName;
        broadcastToChannel(chanName, joinMsg);

        if (channel.getTopic().empty())
            sendMessage(clientFd,
                ":ircserv 331 " + clients[clientFd].getNickname() +
                " " + chanName + " :No topic is set");
        else
            sendMessage(clientFd,
                ":ircserv 332 " + clients[clientFd].getNickname() +
                " " + chanName + " :" + channel.getTopic());

        sendNamesReply(*this, clientFd, channel, clients);
    }
}