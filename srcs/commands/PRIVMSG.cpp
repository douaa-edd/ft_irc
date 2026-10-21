#include "../../include/Server.hpp"

void Server::handlePRIVMSG(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 411 " +
            clients[clientFd].getNickname() + " :No recipient given");
        return;
    }

    std::string text = msg.trailing;

    if (text.empty())
    {
        sendMessage(clientFd, ":ircserv 412 " +
            clients[clientFd].getNickname() + " :No text to send");
        return;
    }

    std::vector<std::string> targets = splitByComma(msg.args[0]);

    for (size_t t = 0; t < targets.size(); ++t)
    {
        if (clients.find(clientFd) == clients.end())
            break;

        const std::string &target  = targets[t];
        std::string        fullMsg = clientPrefix(clientFd) + " " + msg.cmd +
                                     " " + target + " :" + text;

        if (target[0] == '#')
        {
            if (channels.find(target) == channels.end())
            {
                sendMessage(clientFd, ":ircserv 403 " +
                    clients[clientFd].getNickname() + " " + target + " :No such channel");
                continue;
            }

            if (!channels[target].isMember(clientFd))
            {
                sendMessage(clientFd, ":ircserv 404 " +
                    clients[clientFd].getNickname() + " " + target + " :Cannot send to channel");
                continue;
            }

            broadcastToChannel(target, fullMsg, clientFd);

            std::string botReply = _bot.process(clients[clientFd].getNickname(), text);
            if (!botReply.empty())
                broadcastToChannel(target, Bot::PREFIX + " PRIVMSG " + target + " :" + botReply);
        }
        else
        {
            if (target == Bot::NICK)
            {
                std::string botReply = _bot.process(clients[clientFd].getNickname(), text);
                if (botReply.empty())
                    botReply = "I don't understand that. Try !help";
                sendMessage(clientFd, Bot::PREFIX + " PRIVMSG " +
                    clients[clientFd].getNickname() + " :" + botReply);
                continue;
            }

            int targetFd = findFdByNick(target);
            if (targetFd == -1)
            {
                sendMessage(clientFd, ":ircserv 401 " +
                    clients[clientFd].getNickname() + " " + target + " :No such nick");
                continue;
            }

            if (!clients[targetFd].isRegistered())
            {
                sendMessage(clientFd, ":ircserv 451 * " + target + " :has not registered");
                continue;
            }

            sendMessage(targetFd, fullMsg);
        }
    }
}