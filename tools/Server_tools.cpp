#include "../includes/Server.hpp"
#include "../includes/Client.hpp"




void Server::processCommand(int index, std::string message)
{
	std::string command;
    std::string argument;

	if (split(message, ' ', command, argument))
	{
	
		if (command == "PASS")
		{
			// if already registered and try again to register 
			
			if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return;
			}

			// PASS already accepted before (even if not registered)
			if (this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return;
			}

			// missing argument
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "server 461 PASS : Not enough parameters\r\n");
				return;
			}

			// wrong password
			if (argument != this->clients[index]->_password)
			{
				sendError(this->clients[index]->get_fd(), "server 464 : Password incorrect\r\n");
				return;
			}

			// correct password
			this->clients[index]->setPassOk(true);
		}
		else if (command == "NICK")
		{
			if (!this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
				return;
			}
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "server 431 : No nickname given\r\n");
				return;
			}
			// Check if nickname is already in use
			if (isNicknameTaken(argument))
			{
				sendError(this->clients[index]->get_fd(), "server 433 " + argument + " : Nickname is already in use\r\n");
				return ;
			}
			if (pars_nick(argument))
			{
				sendError(this->clients[index]->get_fd(), "server 432 " + argument + " : Erroneous nickname\r\n");
				return ;
			}
			this->clients[index]->setNickname(argument);
		}
		else if (command == "USER")
		{
			// 1) USER after full registration is forbidden
			if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return;
			}

			// 2) PASS must be done first
			if (!this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
				return;
			}

			// 3) USER requires parameters
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
				return;
			}

			// 4) Parse USER arguments (username + realname)
			if (!user_parsing(argument, this->clients[index]))
			{
				sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
				return;
			}

			// 5) Try to complete registration
			if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
			}
		}
		//  add more commands here like JOIN, PART, PRIVMSG, etc. use cmmand and argument variables
	}
	else
	{
		this->clients[index]->setPassOk(false);
		sendError(this->clients[index]->get_fd(), "server 421 " + command + " : Unknown command\r\n");
		
		// Invalid command format
	}

}


void sendError(int fd, const std::string& msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) == -1)
		write(2, "Error sending data to client.\n", 30);
}

bool Server::isNicknameTaken(std::string nickname)
{
	for (int i = 1; i < g_num_fds; i++) {
		if (this->clients[i] && this->clients[i]->getNickname() == nickname) {
			return true;  // Found a match!
		}
	}
	return false;  // Not taken
}