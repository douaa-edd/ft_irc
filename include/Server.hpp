#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <map>
#include <fcntl.h>
#include <signal.h>
#include <cerrno>
#include <cstdlib>
#include "Client.hpp"
#include "Channel.hpp"
#include "Bot.hpp"

struct IRCMessage
{
    std::string              prefix;
    std::string              cmd;
    std::vector<std::string> args;
    std::string              trailing; 
};

class Server
{
    private :
        int                      port;
        std::string              pass;
        int                      listening_socket;
        std::vector<pollfd>      poll_fds;
        std::map<int, Client>    clients;
        std::map<std::string, Channel> channels;
        Bot                             _bot;

        IRCMessage parseMessage(const std::string &line);
        void       handleCommand(int clientFd, const IRCMessage &msg);
        void       tryRegister(int clientFd);
        void       removeClient(int fd);
        void       acceptNewClient();
        void       receiveData(size_t pollIndex);


        void broadcastToChannel(const std::string &chanName,
                                const std::string &line,
                                int excludeFd = -1);

        int  findFdByNick(const std::string &nick) const;

        std::string clientPrefix(int fd) const;
        
        std::vector<std::string> splitByComma(const std::string &s);

    public :
        Server();
        Server(int port, std::string pass, int listening_socket);
        ~Server();

        void run();

        void handlePASS(int clientFd, const IRCMessage &msg);
        void handleNICK(int clientFd, const IRCMessage &msg);
        void handleUSER(int clientFd, const IRCMessage &msg);
        void handleJOIN(int clientFd, const IRCMessage &msg);
        void handlePART(int clientFd, const IRCMessage &msg);
        void handlePRIVMSG(int clientFd, const IRCMessage &msg);
        void handleQUIT(int clientFd, const IRCMessage &msg);
        void handleTOPIC(int clientFd, const IRCMessage &msg);
        void handleINVITE(int clientFd, const IRCMessage &msg);
        void handleKICK(int clientFd, const IRCMessage &msg);
        void handlePING(int clientFd, const IRCMessage &msg);
        void handleMODE(int clientFd, const IRCMessage &msg);

        void sendMessage(int fd, const std::string &msg);
};

#endif
