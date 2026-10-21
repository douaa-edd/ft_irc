#include "../include/Bot.hpp"
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>


const std::string Bot::NICK   = "ircbot";
const std::string Bot::PREFIX = ":ircbot!bot@ircserv";


Bot::Bot()  {}
Bot::~Bot() {}


std::string Bot::process(const std::string &senderNick,
                          const std::string &text) const
{
    if (text.empty() || text[0] != '!')
        return "";

    std::string cmd  = "";
    std::string rest = "";

    size_t space = text.find(' ');
    if (space == std::string::npos)
        cmd = text.substr(1);       
    else
    {
        cmd  = text.substr(1, space - 1); 
        rest = text.substr(space + 1);  
    }

    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(cmd[i])));


    if (cmd == "hello" || cmd == "hi")
    {
        return "Hello, " + senderNick + "! I am ircbot. Type !help to see my commands.";
    }

    if (cmd == "help")
    {
        return "Commands: !hello  !time  !roll  !echo <text>  !help";
    }

    if (cmd == "time")
    {
        time_t now = std::time(NULL);
        std::string t = std::ctime(&now);
        if (!t.empty() && t[t.size() - 1] == '\n')
            t.erase(t.size() - 1);
        return "Current server time: " + t;
    }

    if (cmd == "roll")
    {
        int result = (std::rand() % 6) + 1;
        std::stringstream ss;
        ss << result;
        return senderNick + " rolled a " + ss.str() + "!";
    }

    if (cmd == "echo")
    {
        if (rest.empty())
            return "You did not give me anything to echo!";
        return rest;
    }

    return "";
}
