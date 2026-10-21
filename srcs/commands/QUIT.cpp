#include "../../include/Server.hpp"

void Server::handleQUIT(int clientFd, const IRCMessage &msg)
{
    std::string reason = msg.trailing.empty() ? "Client quit" : msg.trailing;

    std::string nick   = clients[clientFd].getNickname();
    std::string quitMsg = clientPrefix(clientFd) + " QUIT :" + reason;

    std::cout << "[Server] " << (nick.empty() ? "(unnamed)" : nick)
              << " quit (" << reason << ")" << std::endl;

    std::vector<std::string> toDelete;
    for (std::map<std::string, Channel>::iterator it = channels.begin();
         it != channels.end(); ++it)
    {
        Channel &ch = it->second;
        if (ch.isMember(clientFd))
        {
            broadcastToChannel(it->first, quitMsg, clientFd);
            ch.removeMember(clientFd);
            ch.removeOperator(clientFd);
            if (ch.empty())
                toDelete.push_back(it->first);
        }
    }

    for (size_t i = 0; i < toDelete.size(); i++)
        channels.erase(toDelete[i]);

    sendMessage(clientFd, "ERROR :Closing connection (" + reason + ")");

    close(clientFd);
    clients.erase(clientFd);
}