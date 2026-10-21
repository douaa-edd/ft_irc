#include "../include/Client.hpp"

Client::Client()
    : fd(-1),
      nickname(""),
      username(""),
      realname(""),
      hostname(""),
      buffer(""),
      passAccepted(false),
      registered(false)
{
}

Client::Client(int fd)
    : fd(fd),
      nickname(""),
      username(""),
      realname(""),
      hostname(""),
      buffer(""),
      passAccepted(false),
      registered(false)
{
}

Client::~Client() {}

int Client::getFd() const
{
    return fd;
}

std::string Client::getNickname() const
{
    return nickname;
}

std::string Client::getUsername() const
{
    return username;
}

std::string Client::getRealname() const
{
    return realname;
}

std::string Client::getHostname() const
{
    return hostname;
}

std::string &Client::getBuffer()
{
    return buffer;
}

const std::string &Client::getBuffer() const
{
    return buffer;
}

bool Client::isRegistered() const
{
    return registered;
}

bool Client::isPassAccepted() const
{
    return passAccepted;
}

void Client::setNickname(std::string nick)
{
    nickname = nick;
}

void Client::setUsername(std::string user)
{
    username = user;
}

void Client::setRealname(std::string real)
{
    realname = real;
}

void Client::setHostname(std::string host)
{
    hostname = host;
}

void Client::setBuffer(const std::string &buf)
{
    buffer = buf;
}

void Client::appendBuffer(const std::string &data)
{
    buffer += data;
}

void Client::setRegistered(bool reg)
{
    registered = reg;
}

void Client::setPassAccepted(bool accepted)
{
    passAccepted = accepted;
}

std::string Client::getPrefix() const
{
    std::string user = username.empty() ? "unknown" : username;
    std::string host = hostname.empty() ? "localhost" : hostname;
    return nickname + "!" + user + "@" + host;
}