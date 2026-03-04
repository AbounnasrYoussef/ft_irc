NAME = ircserv

CC = c++

CFLAGS = -g3 -fsanitize=address -Iincludes

SRC = main.cpp srcs/Server.cpp srcs/Channel.cpp srcs/Client.cpp \
      tools/Client_tools.cpp tools/Server_tools.cpp tools/privmsg.cpp \
      tools/kick.cpp tools/mode.cpp tools/Join.cpp tools/Topic.cpp \
      tools/Invite.cpp srcs/Bot.cpp tools/tools.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all