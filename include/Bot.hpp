#ifndef BOT_HPP
#define BOT_HPP

#include <string>


class Bot
{
    public:
        static const std::string NICK;
        static const std::string PREFIX;

        Bot();
        ~Bot();
       
        std::string process(const std::string &senderNick,
                            const std::string &text) const;

    private:
        Bot(const Bot &);
        Bot &operator=(const Bot &);
};

#endif
