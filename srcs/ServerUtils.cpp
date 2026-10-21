#include "../include/Server.hpp"


IRCMessage Server::parseMessage(const std::string &line)
{
    IRCMessage  msg;
    std::string s = line;

    while (!s.empty() && (s[s.size() - 1] == '\r' || s[s.size() - 1] == '\n'))
        s.erase(s.size() - 1);

    if (s.empty())
        return msg;

    size_t pos = 0;

    if (s[0] == ':')
    {
        size_t spacePos = s.find(' ');
        if (spacePos == std::string::npos)
            return msg;
        msg.prefix = s.substr(1, spacePos - 1);
        pos = spacePos + 1;
    }

    size_t trailPos = s.find(" :", pos);
    if (trailPos != std::string::npos)
    {
        msg.trailing = s.substr(trailPos + 2);
        s = s.substr(0, trailPos);
    }

    std::stringstream ss(s.substr(pos));
    std::string       word;
    bool              first = true;
    while (ss >> word)
    {
        if (first)
        {
            msg.cmd = word;
            for (size_t i = 0; i < msg.cmd.size(); i++)
                msg.cmd[i] = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(msg.cmd[i])));
            first = false;
        }
        else
            msg.args.push_back(word);
    }

    return msg;
}


void Server::handleCommand(int clientFd, const IRCMessage &msg)
{
    if (msg.cmd.empty())
        return;

    std::string who = clients[clientFd].getNickname();
    if (who.empty())
    {
        std::ostringstream oss;
        oss << clientFd;
        who = "(fd=" + oss.str() + ")";
    }
    std::cout << "[" << who << "] " << msg.cmd;
    for (size_t i = 0; i < msg.args.size(); i++)
        std::cout << " " << msg.args[i];
    if (!msg.trailing.empty())
        std::cout << " :" << msg.trailing;
    std::cout << std::endl;

    if      (msg.cmd == "PASS")   handlePASS(clientFd, msg);
    else if (msg.cmd == "NICK")   handleNICK(clientFd, msg);
    else if (msg.cmd == "USER")   handleUSER(clientFd, msg);
    else if (msg.cmd == "PING")   handlePING(clientFd, msg);
    else if (msg.cmd == "PONG")   return;
    else if (msg.cmd == "CAP")    return;
    else if (msg.cmd == "JOIN")   handleJOIN(clientFd, msg);
    else if (msg.cmd == "PART")   handlePART(clientFd, msg);
    else if (msg.cmd == "PRIVMSG" || msg.cmd == "NOTICE")
                                  handlePRIVMSG(clientFd, msg);
    else if (msg.cmd == "TOPIC")  handleTOPIC(clientFd, msg);
    else if (msg.cmd == "MODE")   handleMODE(clientFd, msg);
    else if (msg.cmd == "INVITE") handleINVITE(clientFd, msg);
    else if (msg.cmd == "KICK")   handleKICK(clientFd, msg);
    else if (msg.cmd == "QUIT")   handleQUIT(clientFd, msg);
    else
    {
        sendMessage(clientFd, ":ircserv 421 " +
            clients[clientFd].getNickname() + " " + msg.cmd + " :Unknown command");
    }
}


void Server::sendMessage(int fd, const std::string &msg)
{
    std::string line = msg + "\r\n";
    ssize_t sent = send(fd, line.c_str(), line.size(), 0);
    if (sent == -1)
        std::cerr << "send() error on fd " << fd << std::endl;
}


void Server::broadcastToChannel(const std::string &chanName,
                                 const std::string &line,
                                 int excludeFd)
{
    if (channels.find(chanName) == channels.end())
        return;
    const std::vector<int> &members = channels[chanName].getMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i] != excludeFd)
            sendMessage(members[i], line);
    }
}


int Server::findFdByNick(const std::string &nick) const
{
    for (std::map<int, Client>::const_iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        if (it->second.getNickname() == nick)
            return it->first;
    }
    return -1;
}


std::string Server::clientPrefix(int fd) const
{
    std::map<int, Client>::const_iterator it = clients.find(fd);
    if (it == clients.end())
        return ":unknown";
    return ":" + it->second.getPrefix();
}


void Server::tryRegister(int clientFd)
{
    Client &client = clients[clientFd];

    if (client.isRegistered())
        return;

    if (!client.isPassAccepted() ||
        client.getNickname().empty() ||
        client.getUsername().empty())
        return;

    client.setRegistered(true);

    std::cout << "[server] " << client.getNickname()
              << " (fd=" << clientFd << ") registered as "
              << client.getPrefix() << std::endl;

    const std::string &nick = client.getNickname();

    sendMessage(clientFd, ":ircserv 001 " + nick +
        " :Welcome to the IRC Network " + client.getPrefix());
    sendMessage(clientFd, ":ircserv 002 " + nick +
        " :Your host is ircserv, running version 1.0");
    sendMessage(clientFd, ":ircserv 003 " + nick +
        " :This server was created today");
    sendMessage(clientFd, ":ircserv 004 " + nick +
        " ircserv 1.0 o itklo");
    sendMessage(clientFd, ":ircserv 005 " + nick +
        " CHANMODES=,k,l,it CHANTYPES=# PREFIX=(o)@ NETWORK=ircserv :are supported");

    sendMessage(clientFd, ":ircserv 375 " + nick + " :- ircserv Message of the day -");
    sendMessage(clientFd, ":ircserv 372 " + nick + " :- Welcome to ft_irc!");
    sendMessage(clientFd, ":ircserv 372 " + nick + " :- Type /join #channel to get started.");
    sendMessage(clientFd, ":ircserv 376 " + nick + " :End of MOTD");
}

void Server::removeClient(int fd)
{
    if (clients.find(fd) == clients.end())
        return;

    std::string prefix = clientPrefix(fd);
    std::string nick   = clients[fd].getNickname();

    std::cout << "[server] " << (nick.empty() ? "(unnamed)" : nick)
              << " (fd=" << fd << ") disconnected" << std::endl;

    std::vector<std::string> toDelete;
    for (std::map<std::string, Channel>::iterator it = channels.begin();
         it != channels.end(); ++it)
    {
        Channel &ch = it->second;
        if (ch.isMember(fd))
        {
            broadcastToChannel(it->first, prefix + " QUIT :connection closed", fd);
            ch.removeMember(fd);
            ch.removeOperator(fd);
            if (ch.empty())
                toDelete.push_back(it->first);
        }
    }

    for (size_t i = 0; i < toDelete.size(); i++)
        channels.erase(toDelete[i]);

    close(fd);
    clients.erase(fd);
}


std::vector<std::string> Server::splitByComma(const std::string &s)
{
    std::vector<std::string> result;
    std::string              token;
    for (size_t i = 0; i <= s.size(); ++i)
    {
        if (i == s.size() || s[i] == ',')
        {
            if (!token.empty())
            {
                result.push_back(token);
                token.clear();
            }
        }
        else
            token += s[i];
    }
    return result;
}