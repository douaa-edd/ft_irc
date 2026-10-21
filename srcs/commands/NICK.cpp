#include "../../include/Server.hpp"

static bool isValidNick(const std::string &nick)
{
    if (nick.empty() || nick.size() > 30)
        return false;

    char first = nick[0];
    if (std::isdigit(static_cast<unsigned char>(first)) ||
        first == '#' || first == ':' || first == ' ')
        return false;

    for (size_t i = 0; i < nick.size(); i++)
    {
        char c = nick[i];
        if (!std::isalnum(static_cast<unsigned char>(c)) &&
            c != '-' && c != '_' && c != '[' && c != ']' &&
            c != '{' && c != '}' && c != '|' && c != '\\' &&
            c != '^' && c != '`')
            return false;
    }
    return true;
}

void Server::handleNICK(int clientFd, const IRCMessage &msg)
{
    if (!pass.empty() && !clients[clientFd].isPassAccepted() && !clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 464 * :Password required");
        return;
    }

   
    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 431 * :No nickname given");
        return;
    }
    if (msg.args.size() > 1)
    {
        sendMessage(clientFd, ":ircserv 432 " + msg.args[0] + " :Erroneous nickname");
        return;
    }

    std::string newNick = msg.args[0];

    if (newNick.empty())
    {
        sendMessage(clientFd, ":ircserv 431 * :No nickname given");
        return;
    }

    if (!isValidNick(newNick))
    {
        sendMessage(clientFd, ":ircserv 432 " + newNick + " :Erroneous nickname");
        return;
    }

    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        if (it->first != clientFd && it->second.getNickname() == newNick)
        {
            sendMessage(clientFd, ":ircserv 433 * " + newNick + " :Nickname is already in use");
            return;
        }
    }

    std::string oldPrefix = clientPrefix(clientFd);
    std::string oldNick   = clients[clientFd].getNickname();
    clients[clientFd].setNickname(newNick);

    if (clients[clientFd].isRegistered())
    {
        std::string nickMsg = oldPrefix + " NICK " + newNick;

        sendMessage(clientFd, nickMsg);

        std::vector<int> notified;
        notified.push_back(clientFd);

        for (std::map<std::string, Channel>::iterator ch = channels.begin();
             ch != channels.end(); ++ch)
        {
            if (!ch->second.isMember(clientFd))
                continue;
            const std::vector<int> &members = ch->second.getMembers();
            for (size_t i = 0; i < members.size(); i++)
            {
                bool already = false;
                for (size_t j = 0; j < notified.size(); j++)
                {
                    if (notified[j] == members[i])
                    { already = true; break; }
                }
                if (!already)
                {
                    sendMessage(members[i], nickMsg);
                    notified.push_back(members[i]);
                }
            }
        }
    }

    tryRegister(clientFd);
}