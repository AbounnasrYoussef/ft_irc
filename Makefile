NAME = ircserv

CC = c++

CFLAGS = #-Wall -Wextra -Werror -std=c++98

<<<<<<< HEAD
SRC = main.cpp srcs/Server.cpp srcs/Client.cpp tools/Client_tools.cpp tools/Server_tools.cpp \
		srcs/Channel.cpp
=======
SRC = main.cpp srcs/Server.cpp  srcs/Channel.cpp  srcs/Client.cpp tools/Client_tools.cpp tools/Server_tools.cpp tools/privmsg.cpp  tools/kick.cpp tools/mode.cpp
>>>>>>> youssef

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ) 
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

<<<<<<< HEAD
%.o: %.cpp includes/Channel.hpp includes/Client.hpp  includes/Server.hpp 
=======
%.o: %.cpp includes/Channel.hpp includes/Client.hpp includes/Commands.hpp includes/Server.hpp
>>>>>>> youssef
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all