NAME    = ircserv
CC      = c++
CFLAGS  = -Wall -Wextra -Werror -std=c++98

SRCS    = \
	srcs/main.cpp            \
	srcs/Server.cpp          \
	srcs/ServerUtils.cpp     \
	srcs/Client.cpp          \
	srcs/Channel.cpp         \
	srcs/Bot.cpp             \
	srcs/commands/PASS.cpp   \
	srcs/commands/NICK.cpp   \
	srcs/commands/USER.cpp   \
	srcs/commands/JOIN.cpp   \
	srcs/commands/PART.cpp   \
	srcs/commands/PRIVMSG.cpp \
	srcs/commands/QUIT.cpp   \
	srcs/commands/TOPIC.cpp  \
	srcs/commands/INVITE.cpp \
	srcs/commands/KICK.cpp   \
	srcs/commands/PING.cpp   \
	srcs/commands/MODE.cpp

OBJS = $(SRCS:.cpp=.o)

HEADERS = include/Server.hpp include/Client.hpp include/Channel.hpp include/Bot.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re