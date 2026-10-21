#include "../include/Channel.hpp"
#include <sstream>

Channel::Channel()
    : name(""), topic(""), inviteOnly(false),
      topicProtected(false), key(""), userLimit(-1)
{
}

Channel::Channel(const std::string &name)
    : name(name), topic(""), inviteOnly(false),
      topicProtected(false), key(""), userLimit(-1)
{
}

Channel::~Channel() {}


std::string Channel::getName() const
{
    return name;
}

std::string Channel::getTopic() const
{
    return topic;
}

const std::vector<int> &Channel::getMembers() const
{
    return members;
}

bool Channel::isInviteOnly() const
{
    return inviteOnly;
}

bool Channel::isTopicProtected() const
{
    return topicProtected;
}

std::string Channel::getKey() const
{
    return key;
}

int Channel::getUserLimit() const
{
    return userLimit;
}

int Channel::getMemberCount() const
{
    return static_cast<int>(members.size());
}


void Channel::setTopic(const std::string &t)
{
    topic = t;
}

void Channel::setInviteOnly(bool value)
{
    inviteOnly = value;
}

void Channel::setTopicProtected(bool value)
{
    topicProtected = value;
}

void Channel::setKey(const std::string &k)
{
    key = k;
}

void Channel::setUserLimit(int limit)
{
    userLimit = limit;
}


void Channel::addMember(int fd)
{
    if (!isMember(fd))
        members.push_back(fd);
}

void Channel::removeMember(int fd)
{
    std::vector<int>::iterator it = std::find(members.begin(), members.end(), fd);
    if (it != members.end())
        members.erase(it);
}

void Channel::addOperator(int fd)
{
    if (!isOperator(fd))
        operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
    std::vector<int>::iterator it = std::find(operators.begin(), operators.end(), fd);
    if (it != operators.end())
        operators.erase(it);
}

void Channel::addInvited(int fd)
{
    if (!isInvited(fd))
        invited.push_back(fd);
}

void Channel::removeInvited(int fd)
{
    std::vector<int>::iterator it = std::find(invited.begin(), invited.end(), fd);
    if (it != invited.end())
        invited.erase(it);
}


bool Channel::isMember(int fd) const
{
    return std::find(members.begin(), members.end(), fd) != members.end();
}

bool Channel::isOperator(int fd) const
{
    return std::find(operators.begin(), operators.end(), fd) != operators.end();
}

bool Channel::isInvited(int fd) const
{
    return std::find(invited.begin(), invited.end(), fd) != invited.end();
}

bool Channel::empty() const
{
    return members.empty();
}

std::string Channel::getModeString() const
{
    std::string modes = "+";
    std::string params = "";

    if (inviteOnly)
        modes += "i";
    if (topicProtected)
        modes += "t";
    if (!key.empty())
    {
        modes += "k";
        params += " " + key;
    }
    if (userLimit != -1)
    {
        modes += "l";
        std::stringstream ss;
        ss << userLimit;
        params += " " + ss.str();
    }
    if (modes == "+")
        modes = "";
    return modes + params;
}
