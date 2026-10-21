#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>

class Channel
{
    private :
        std::string      name;
        std::string      topic;
        std::vector<int> members;
        std::vector<int> operators;
        std::vector<int> invited;
        bool             inviteOnly;   
        bool             topicProtected;
        std::string      key;
        int              userLimit;

    public :
        Channel();
        Channel(const std::string &name);
        ~Channel();

        std::string           getName()     const;
        std::string           getTopic()    const;
        const std::vector<int> &getMembers() const;
        bool                  isInviteOnly()     const;
        bool                  isTopicProtected() const;
        std::string           getKey()      const;
        int                   getUserLimit() const;
        int                   getMemberCount() const;

        void setTopic(const std::string &topic);
        void setInviteOnly(bool value);
        void setTopicProtected(bool value);
        void setKey(const std::string &key);
        void setUserLimit(int limit);

        void addMember(int fd);
        void removeMember(int fd);
        void addOperator(int fd);
        void removeOperator(int fd);
        void addInvited(int fd);
        void removeInvited(int fd);

        bool isMember(int fd)   const;
        bool isOperator(int fd) const;
        bool isInvited(int fd)  const;
        bool empty()            const;

        std::string getModeString() const;
};

#endif