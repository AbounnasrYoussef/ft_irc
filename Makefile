NAME = ircserv

CC = c++

CFLAGS = #-Wall -Wextra -Werror -std=c++98

SRC = main.cpp srcs/Server.cpp  srcs/Channel.cpp  srcs/Client.cpp tools/Client_tools.cpp tools/Server_tools.cpp tools/privmsg.cpp  tools/kick.cpp tools/mode.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ) 
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp includes/Channel.hpp includes/Client.hpp includes/Commands.hpp includes/Server.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all