#include "../../include/Server.hpp"

void Server::handleTOPIC(int clientFd, const IRCMessage &msg)
{
    if (!clients[clientFd].isRegistered())
    {
        sendMessage(clientFd, ":ircserv 451 * :You have not registered");
        return;
    }

    if (msg.args.empty())
    {
        sendMessage(clientFd, ":ircserv 461 TOPIC :Not enough parameters");
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

    
    bool        isSet    = false;
    std::string newTopic = "";

    if (!msg.trailing.empty())
    {
        isSet    = true;
        newTopic = msg.trailing;
    }
    else if (msg.args.size() >= 2)
    {
        isSet    = true;
        newTopic = msg.args[1];
    }

    if (!isSet)
    {
        std::string tp = channel.getTopic();
        if (tp.empty())
            sendMessage(clientFd, ":ircserv 331 " +
                clients[clientFd].getNickname() + " " + chanName + " :No topic is set");
        else
            sendMessage(clientFd, ":ircserv 332 " +
                clients[clientFd].getNickname() + " " + chanName + " :" + tp);
        return;
    }

  
    if (channel.isTopicProtected() && !channel.isOperator(clientFd))
    {
        sendMessage(clientFd, ":ircserv 482 " +
            clients[clientFd].getNickname() + " " + chanName + " :You're not channel operator");
        return;
    }

    channel.setTopic(newTopic);

    std::string topicMsg = clientPrefix(clientFd) + " TOPIC " + chanName + " :" + newTopic;
    broadcastToChannel(chanName, topicMsg);
}