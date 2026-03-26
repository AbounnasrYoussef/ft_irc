#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Commands.hpp"
#include <algorithm>
#include <vector>

bool Server::check_passok(std::string command, std::string argument, int index)
{
	if (command == "PASS")
	{
		if (this->clients[index]->isRegistered())
		{
			sendError(this->clients[index]->get_fd(), "462 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :You may not reregister\r\n");
			return false;
		}
		if (this->clients[index]->isPassOk())
		{
			sendError(this->clients[index]->get_fd(), "462 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :You may not reregister\r\n");
			return false;
		}
		if (argument.empty())
		{
			sendError(this->clients[index]->get_fd(), "461 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" PASS :Not enough parameters\r\n");
			return false;
		}
		if (argument != this->password)
		{
			sendError(this->clients[index]->get_fd(), "464 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :Password incorrect\r\n");
			return false;
		}
	}
	return true;
}

bool Server::check_authentication(std::string command, std::string argument, int index)
{

	if (command == "NICK")
	{
		if (argument.empty())
		{
			sendError(this->clients[index]->get_fd(), "431 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :No nickname given\r\n");
			return false;
		}
		
		if (isNicknameTaken(argument, index))
		{
			sendError(this->clients[index]->get_fd(), "433 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" " + argument + " :Nickname is already in use\r\n");
			return false;
		}
		
		if (pars_nick(argument) == false)
		{
			sendError(this->clients[index]->get_fd(), "432 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" " + argument + " :Erroneous nickname\r\n");
			return false;
		}
		
		if (this->clients[index]->isRegistered())
		{
			std::string oldNick = this->clients[index]->getNickname();
			this->clients[index]->setNickname(argument);
			sendError(this->clients[index]->get_fd(), ":" + oldNick + "!" + 
				this->clients[index]->getUsername() + "@" + 
				this->clients[index]->getIP() + " NICK :" + argument + "\r\n");
			return true;
		}
		
		this->clients[index]->setNickname(argument);
		return true;
	}
	else if (command == "USER")
	{
		if (this->clients[index]->isRegistered())
		{
			sendError(this->clients[index]->get_fd(), "462 " + this->clients[index]->getNickname() + 
				" :You may not reregister\r\n");
			return false;
		}

		if (this->clients[index]->isUserSet())
		{
			sendError(this->clients[index]->get_fd(), "462 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :You may not reregister\r\n");
			return false;
		}

		if (argument.empty())
		{
			sendError(this->clients[index]->get_fd(), "461 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" USER :Not enough parameters \r\n");
		
			return false;
		}

		if (!user_parsing(argument, this->clients[index]))
		{
			sendError(this->clients[index]->get_fd(), "461 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" USER :Not enough parameters\r\n");
			return false;
		}

		this->clients[index]->setUserSet(true);
		return true;
	}
	return true;
}

void Server::processCommand(int index, std::string &message)
{
	std::string command;
	std::string argument;

	if (split(message, command, argument))
	{
		ft_toupper(command);
		if (command == "PASS" || command == "NICK" || command == "USER")
		{

			if (command == "PASS")
			{
				if (check_passok(command, argument, index))
				{
					this->clients[index]->setPassOk(true);
					if (this->clients[index]->isRegistered() && this->clients[index]->isWelcomeSent() == false)
					{
						sendError(this->clients[index]->get_fd(), "001 " + this->clients[index]->getNickname() + 
							" :Welcome to the Internet Relay Network " +
							this->clients[index]->getNickname() + "!" +
							this->clients[index]->getUsername() + "@" + this->clients[index]->getIP() + "\r\n");
						this->clients[index]->setWelcomeSent(true);
					}
				}
				else
				{
					message = "";
				}
				return;
			}

			if (check_authentication(command, argument, index) == false)
			{
				message = "";
				return;
			}
			if (this->clients[index]->isRegistered() && this->clients[index]->isWelcomeSent() == false)
			{
				sendError(this->clients[index]->get_fd(), "001 " + this->clients[index]->getNickname() + 
					" :Welcome to the Internet Relay Network " +
					this->clients[index]->getNickname() + "!" +
					this->clients[index]->getUsername() + "@" + this->clients[index]->getIP() + "\r\n");
				this->clients[index]->setWelcomeSent(true);
			}
			return;
		}
		else if (this->clients[index]->isRegistered() == false)
		{
			sendError(this->clients[index]->get_fd(), "451 " + 
				(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
				" :You have not registered\r\n");
			message = "";
			return;
		}
		else if (command == "JOIN")
			this->handel_Join(command, argument, index);
		else if (command == "TOPIC")
			this->handel_Topic(command, argument, index);
		else if (command == "INVITE")
			this->handel_Invite(command, argument, index);
		else if (command == "PRIVMSG")
			this->handle_privmsg(index, argument);
		else if (command == "KICK")
			this->handle_kick(index, argument);
		else if (command == "MODE")
			handle_mode(index, argument);
		else if (command == "QUIT")
			handle_quit(index);
		else
			sendError(this->clients[index]->get_fd(), "421 " + command + " :Unknown command\r\n");
	}
}

void sendError(int fd, const std::string &msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) == -1)
		std::cerr << "Error sending data to client." << std::endl;
}

bool Server::isNicknameTaken(std::string nickname, int excludeIndex)
{
    for (int i = 1; i < (int)this->_fds.size(); i++)
    {
        if (this->clients[i] && i != excludeIndex && this->clients[i]->getNickname() == nickname) 
        {
            return true;  // Found a match in another client!
        }
    }
    return false;  // Not taken
}

int Server::getServerFd()
{
    return this->server_Fd;
}


void Server::handle_quit(int index)
{
	if (!this->clients[index]->isRegistered())
	{
		sendError(this->clients[index]->get_fd(), "451 " + 
			(this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
			" :You have not registered\r\n");
		return;
	}
	removeClient(this->_fds, clients, index);
}

void Server::Quit()
{
	for (size_t i = 1; i < this->clients.size(); ++i)
	{
		if (this->clients[i])
		{
			close(this->clients[i]->get_fd());
			delete this->clients[i];
			this->clients[i] = NULL;
		}
	}
	clients.clear();
	_fds.clear();
	
	// Delete all channels
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); 
		it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	
	if (this->server_Fd != -1)
	{
		close(this->server_Fd);
		this->server_Fd = -1;
	}
}

void ft_toupper(std::string &str)
{
    size_t i = 0;
    size_t len = str.length();
    
    while (i < len)
    {
        str[i] = std::toupper(str[i]);
        i++;
    }

}
