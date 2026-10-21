#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
    private :
        int         fd;
        std::string nickname;
        std::string username;
        std::string realname;
        std::string hostname;
        std::string buffer;
        bool        passAccepted;
        bool        registered;

    public :
        Client();
        Client(int fd);
        ~Client();

        int         getFd()       const;
        std::string getNickname() const;
        std::string getUsername() const;
        std::string getRealname() const;
        std::string getHostname() const;
        std::string       &getBuffer();
        const std::string &getBuffer() const;
        bool        isRegistered()   const;
        bool        isPassAccepted() const;

        void setNickname(std::string nickname);
        void setUsername(std::string username);
        void setRealname(std::string realname);
        void setHostname(std::string hostname);
        void setBuffer(const std::string &buf);
        void appendBuffer(const std::string &data);
        void setRegistered(bool registered);
        void setPassAccepted(bool passAccepted);

        std::string getPrefix() const;
};

#endif