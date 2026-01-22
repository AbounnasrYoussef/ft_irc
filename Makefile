NAME = ircserv

CC = c++

CFLAGS = -g3 -fsanitize=address #-Wall -Wextra -Werror -std=c++98

SRC = main.cpp srcs/Server.cpp srcs/Client.cpp tools/Client_tools.cpp tools/Server_tools.cpp srcs/Bot.cpp tools/tools.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp  includes/Server.hpp includes/Client.hpp includes/Channel.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all