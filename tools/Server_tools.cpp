#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


bool Server::check_passok(std::string command, std::string argument, int index)
{
	// if (command == "PASS" && !argument.empty())
	// 	return true;
	if (command == "PASS")
		{
			// if already registered and try again to register 
			
			if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return false;
			}

			// PASS already accepted before (even if not registered)
			if (this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return false;
			}

			// missing argument
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "server 461 PASS : Not enough parameters\r\n");
				return false;
			}

			// wrong password
			if (argument != this->password)
			{
				sendError(this->clients[index]->get_fd(), "server 464 : Password incorrect\r\n");
				return false;
			}
			}
	return true;
}

bool Server::check_authentication(std::string command, std::string argument, int index)
{
	if (command != "NICK" && command != "USER")
	{
		// sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
		return false;
	}
	if (command == "NICK")
	{
		std::cout << "Processing NICK command with argument: " << argument << std::endl; // Debug line
		if (!this->clients[index]->isPassOk())
		{
			sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
			return false;
		}
		if (argument.empty())
		{
			sendError(this->clients[index]->get_fd(), "server 431 : No nickname given\r\n");
			return false;
		}
		// Check if nickname is already in use (excluding current client)
		if (isNicknameTaken(argument, index))
		{
			sendError(this->clients[index]->get_fd(), "server 433 " + argument + " : Nickname is already in use\r\n");
			return false;
		}
		if (pars_nick(argument))
		{
			sendError(this->clients[index]->get_fd(), "server 432 " + argument + " : Erroneous nickname\r\n");
			return false;
		}
		this->clients[index]->setNickname(argument);
		return true;
		// Check if registration is now complete (USER was already done)
		// if (this->clients[index]->isRegistered())
		// {
		// 	sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
		// }
	}
	else if (command == "USER")
	{
		if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
				return false;
			}

			// 2) PASS must be done first
			if (!this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
				return false;
			}

			// 3) USER requires parameters
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
				return false;
			}

			// 4) Parse USER arguments (username + realname)
			if (!user_parsing(argument, this->clients[index]))
			{
				sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
				return false;
			}

			// 5) Try to complete registration
			// if (this->clients[index]->isRegistered())
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
			// }
			return true;
	}
	return true;
}


void Server::processCommand(int index, std::string message)
{
	std::string command;
    std::string argument;

	if (split(message, ' ', command, argument))
	{
		
		if (clients[index]->isRegistered() == false)
		{
			if (clients[index]->isPassOk() == false)
			{
				if (check_passok(command, argument, index))
				{
					this->clients[index]->setPassOk(true);
					return;
				}
				return;
			}
		}
		if (check_authentication(command, argument, index) == false)
			return;
		// clients[index]->isRegistered(true);
		if (this->clients[index]->isRegistered())
		{
			sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
		}
		return;
	}

		// if (command == "PASS")
		// {
		// 	// if already registered and try again to register 
			
		// 	if (this->clients[index]->isRegistered())
		// 	{
		// 		sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
		// 		return;
		// 	}

		// 	// PASS already accepted before (even if not registered)
		// 	if (this->clients[index]->isPassOk())
		// 	{
		// 		sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
		// 		return;
		// 	}

		// 	// missing argument
		// 	if (argument.empty())
		// 	{
		// 		sendError(this->clients[index]->get_fd(), "server 461 PASS : Not enough parameters\r\n");
		// 		return;
		// 	}

		// 	// wrong password
		// 	if (argument != this->password)
		// 	{
		// 		sendError(this->clients[index]->get_fd(), "server 464 : Password incorrect\r\n");
		// 		return;
		// 	}

			// correct password
		// }
		// else if (command == "NICK")
		// {
			
		// }
		// else if (command == "USER")
		// {
			// 1) USER after full registration is forbidden
			// if (this->clients[index]->isRegistered())
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 462 : You may not reregister\r\n");
			// 	return;
			// }

			// // 2) PASS must be done first
			// if (!this->clients[index]->isPassOk())
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 451 : You have not registered\r\n");
			// 	return;
			// }

			// // 3) USER requires parameters
			// if (argument.empty())
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
			// 	return;
			// }

			// // 4) Parse USER arguments (username + realname)
			// if (!user_parsing(argument, this->clients[index]))
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 461 USER : Not enough parameters\r\n");
			// 	return;
			// }

			// // 5) Try to complete registration
			// if (this->clients[index]->isRegistered())
			// {
			// 	sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
			// }
	// 	}
	// 	//  add more commands here like JOIN, PART, PRIVMSG, etc. use cmmand and argument variables
	// }
	// else
	// {
	// 	// this->clients[index]->setPassOk(false);    
	// 	sendError(this->clients[index]->get_fd(), "server 421 " + command + " : Unknown command\r\n");
		
	// 	// Invalid command format
	// }

}


void sendError(int fd, const std::string& msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) == -1)
		write(2, "Error sending data to client.\n", 30);
}

bool Server::isNicknameTaken(std::string nickname, int excludeIndex)
{
	for (int i = 1; i < g_num_fds; i++) {
		if (this->clients[i] && i != excludeIndex && this->clients[i]->getNickname() == nickname) {
			return true;  // Found a match in another client!
		}
	}
	return false;  // Not taken
}