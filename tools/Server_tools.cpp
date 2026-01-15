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
				sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
				return false;
			}

			// PASS already accepted before (even if not registered)
			if (this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
				return false;
			}

			// missing argument
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS PASS : Not enough parameters\r\n");
				return false;
			}

			// wrong password
			if (argument != this->password)
			{
				// std::cout << "Provided password: [" << argument << "], Expected password: [" << this->password << "]" << std::endl; // Debug line
				sendError(this->clients[index]->get_fd(), "464 ERR_PASSWORDDISALLOWED : Password incorrect\r\n");
				return false;
			}
			}
	return true;
}

bool Server::check_authentication(std::string command, std::string argument, int index)
{

	if (command == "NICK")
	{
		if (!this->clients[index]->isPassOk())
		{
			sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
			return false;
		}
		if (argument.empty())
		{
			sendError(this->clients[index]->get_fd(), "431 ERR_NONICKNAMEGIVEN : No nickname given\r\n");
			return false;
		}
		// Check if nickname is already in use (excluding current client)
		if (isNicknameTaken(argument, index))
		{
			sendError(this->clients[index]->get_fd(), "433 ERR_NICKNAMEINUSE " + argument + " : Nickname is already in use\r\n");
			return false;
		}
		if (pars_nick(argument) == false)
		{
			sendError(this->clients[index]->get_fd(), "432 ERR_ERRONEUSNICKNAME " + argument + " : Erroneous nickname\r\n");
			return false;
		}
		this->clients[index]->setNickname(argument);
		return true;
	}
	else if (command == "USER")
	{
			//  USER after full registration is forbidden
		if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
				return false;
			}

			//  PASS must be done first
			if (!this->clients[index]->isPassOk())
			{
				sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
				return false;
			}

			//  USER requires parameters
			if (argument.empty())
			{
				sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS USER : Not enough parameters\r\n");
				return false;
			}

			// Parse USER arguments (username + realname)
			if (!user_parsing(argument, this->clients[index]))
			{
				sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS USER : Not enough parameters\r\n");
				return false;
			}

			return true;
	}
	return true;
}

void Server::processCommand(int index, std::string &message)
{
	std::string command;
    std::string argument;

	if (split(message, ' ', command, argument))
	{
		// Handle PASS, NICK, USER for registration
		if (command == "PASS" || command == "NICK" || command == "USER")
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
			if (this->clients[index]->isRegistered())
			{
				sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
			}
			return;
		}
		else if (this->clients[index]->isRegistered() == false)
		{
			sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
			return;
		}
		//  add more commands here like JOIN, PART, PRIVMSG, etc. use cmmand and argument variables

		// youssef part : "kick , privmsg , mode"
		else if( command == "PRIVMSG" || command == "KICK" || command == "MODE")
		{
			if (command == "PRIVMSG")
			{
				//handel_privmsg(argument);
			}
			else if (command == "KICK")
			{
				//handel_kick(argument);
			}
			else if (command == "MODE")
			{
				//handel_mode(argument);
			}
		}

	}

}


void sendError(int fd, const std::string& msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) == -1)
		write(2, "Error sending data to client.\n", 30);
}

bool Server::isNicknameTaken(std::string nickname, int excludeIndex)
{
	for (int i = 1; i < g_num_fds; i++)
	{
		if (this->clients[i] && i != excludeIndex && this->clients[i]->getNickname() == nickname) 
		{
			return true;  // Found a match in another client!
		}
	}
	return false;  // Not taken
}