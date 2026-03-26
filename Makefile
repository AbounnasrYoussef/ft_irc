NAME=ircserv

BONUS_NAME = ircserv_bonus

CC=c++
FLAGS= -Wall -Wextra -Werror -std=c++98

SRC= Mandatory/main.cpp Mandatory/srcs/Server.cpp  Mandatory/srcs/Channel.cpp  Mandatory/srcs/Client.cpp Mandatory/tools/Client_tools.cpp Mandatory/tools/Server_tools.cpp Mandatory/tools/privmsg.cpp  Mandatory/tools/kick.cpp Mandatory/tools/mode.cpp \
			Mandatory/tools/Join.cpp Mandatory/tools/Topic.cpp Mandatory/tools/Invite.cpp

Bonus_SRC= Bonus/main.cpp Bonus/srcs/Server.cpp  Bonus/srcs/Channel.cpp  Bonus/srcs/Client.cpp Bonus/tools/Client_tools.cpp Bonus/tools/Server_tools.cpp Bonus/tools/privmsg.cpp  Bonus/tools/kick.cpp Bonus/tools/mode.cpp \
			Bonus/tools/Join.cpp Bonus/tools/Topic.cpp Bonus/tools/Invite.cpp Bonus/srcs/Bot.cpp 


OBJ_SRC=$(SRC:.cpp=.o)
BONUS_OBJ_SRC=$(Bonus_SRC:.cpp=.o)

Mandatory/%.o: Mandatory/%.cpp Mandatory/includes/Channel.hpp Mandatory/includes/Client.hpp  Mandatory/includes/Server.hpp Mandatory/includes/Commands.hpp
	$(CC) $(FLAGS) -c $< -o $@

Bonus/%.o: Bonus/%.cpp Bonus/includes/Channel.hpp Bonus/includes/Client.hpp  Bonus/includes/Server.hpp Bonus/includes/Commands.hpp
	$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

bonus: $(BONUS_NAME)

$(NAME): $(OBJ_SRC)
	$(CC) $(FLAGS) $(OBJ_SRC) -o $(NAME)

$(BONUS_NAME): $(BONUS_OBJ_SRC)
	$(CC) $(FLAGS) $(BONUS_OBJ_SRC) -o $(BONUS_NAME)

clean:
	rm -f $(OBJ_SRC) $(BONUS_OBJ_SRC)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all
