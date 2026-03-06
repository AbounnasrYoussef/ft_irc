#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Commands.hpp"
#include <algorithm>
#include <vector>

// OLD CODE - Error messages without proper nickname in response
// bool Server::check_passok(std::string command, std::string argument, int index)
// {
//     if (command == "PASS")
//     {
//         if (this->clients[index]->isRegistered())
//         {
//             sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
//             return false;
//         }
//         if (this->clients[index]->isPassOk())
//         {
//             sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
//             return false;
//         }
//         if (argument.empty())
//         {
//             sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS PASS : Not enough parameters\r\n");
//             return false;
//         }
//         if (argument != this->password)
//         {
//             sendError(this->clients[index]->get_fd(), "464 ERR_PASSWORDDISALLOWED : Password incorrect\r\n");
//             return false;
//         }
//     }
//     return true;
// }

// NEW CODE - Proper RFC 1459 error format with nickname
bool Server::check_passok(std::string command, std::string argument, int index)
{
    if (command == "PASS")
    {
        // if already registered and try again to register
        if (this->clients[index]->isRegistered())
        {
            sendError(this->clients[index]->get_fd(), "462 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " :You may not reregister\r\n");
            return false;
        }

        // PASS already accepted before (even if not registered)
        if (this->clients[index]->isPassOk())
        {
            sendError(this->clients[index]->get_fd(), "462 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " :You may not reregister\r\n");
            return false;
        }

        // missing argument
        if (argument.empty())
        {
            sendError(this->clients[index]->get_fd(), "461 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " PASS :Not enough parameters\r\n");
            return false;
        }

        // wrong password
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

// OLD CODE - Strict error messages without proper RFC format
// bool Server::check_authentication(std::string command, std::string argument, int index)
// {
//     if (command == "NICK")
//     {
//         if (!this->clients[index]->isPassOk())
//         {
//             sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
//             return false;
//         }
//         if (argument.empty())
//         {
//             sendError(this->clients[index]->get_fd(), "431 ERR_NONICKNAMEGIVEN : No nickname given\r\n");
//             return false;
//         }
//         if (isNicknameTaken(argument, index))
//         {
//             sendError(this->clients[index]->get_fd(), "433 ERR_NICKNAMEINUSE " + argument + " : Nickname is already in use\r\n");
//             return false;
//         }
//         if (pars_nick(argument) == false)
//         {
//             sendError(this->clients[index]->get_fd(), "432 ERR_ERRONEUSNICKNAME " + argument + " : Erroneous nickname\r\n");
//             return false;
//         }
//         this->clients[index]->setNickname(argument);
//         return true;
//     }
//     else if (command == "USER")
//     {
//         if (this->clients[index]->isRegistered())
//         {
//             sendError(this->clients[index]->get_fd(), "462 ERR_ALREADYREGISTRED : You may not reregister\r\n");
//             return false;
//         }
//         if (!this->clients[index]->isPassOk())
//         {
//             sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
//             return false;
//         }
//         if (argument.empty())
//         {
//             sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS USER : Not enough parameters\r\n");
//             return false;
//         }
//         if (!user_parsing(argument, this->clients[index]))
//         {
//             sendError(this->clients[index]->get_fd(), "461 ERR_NEEDMOREPARAMS USER : Not enough parameters\r\n");
//             return false;
//         }
//         return true;
//     }
//     return true;
// }

// NEW CODE - RFC 1459 compliant authentication
// - NICK can be used before PASS (flexible order)
// - NICK change after registration supported
// - USER can only be sent once
// - Proper error message format with nickname
bool Server::check_authentication(std::string command, std::string argument, int index)
{

    if (command == "NICK")
    {
        // NICK can be used before PASS (RFC 1459 allows flexible order)
        // We still need PASS before completing registration, but NICK can be set first
        
        if (argument.empty())
        {
            // Use current nickname or "*" if not set
            sendError(this->clients[index]->get_fd(), "431 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " :No nickname given\r\n");
            return false;
        }
        
        // Check if nickname is already in use (excluding current client)
        if (isNicknameTaken(argument, index))
        {
            sendError(this->clients[index]->get_fd(), "433 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " " + argument + " :Nickname is already in use\r\n");
            return false;
        }
        
        // Validate nickname format according to RFC 1459
        if (pars_nick(argument) == false)
        {
            sendError(this->clients[index]->get_fd(), "432 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " " + argument + " :Erroneous nickname\r\n");
            return false;
        }
        
        // NEW: If client is already registered, this is a NICK change
        if (this->clients[index]->isRegistered())
        {
            std::string oldNick = this->clients[index]->getNickname();
            this->clients[index]->setNickname(argument);
            // Send confirmation of nick change: :oldnick!user@host NICK :newnick
            sendError(this->clients[index]->get_fd(), ":" + oldNick + "!" + 
                this->clients[index]->getUsername() + "@" + 
                this->clients[index]->getIP() + " NICK :" + argument + "\r\n");
            return true;
        }
        
        // Set nickname (registration not complete yet)
        this->clients[index]->setNickname(argument);
        return true;
    }
    else if (command == "USER")
    {
        // USER after full registration is forbidden
        if (this->clients[index]->isRegistered())
        {
            sendError(this->clients[index]->get_fd(), "462 " + this->clients[index]->getNickname() + 
                " :You may not reregister\r\n");
            return false;
        }

        // NEW: USER already sent (even if not fully registered)
        if (this->clients[index]->isUserSet())
        {
            sendError(this->clients[index]->get_fd(), "462 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " :You may not reregister\r\n");
            return false;
        }

        // USER requires parameters
        if (argument.empty())
        {
            sendError(this->clients[index]->get_fd(), "461 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " USER :Not enough parameters\r\n");
            return false;
        }

        // Parse USER arguments (username + realname)
        if (!user_parsing(argument, this->clients[index]))
        {
            sendError(this->clients[index]->get_fd(), "461 " + 
                (this->clients[index]->getNickname().empty() ? "*" : this->clients[index]->getNickname()) + 
                " USER :Not enough parameters\r\n");
            return false;
        }

        // NEW: Mark USER as set
        this->clients[index]->setUserSet(true);
        return true;
    }
    return true;
}
// std::vector<std::string> split_chanel(const std::string &str, char delemeter)
// {
//     std::vector<std::string> resolt_chanel;

//     int start = 0;
//     int end;
//     while ((end = str.find(delemeter, start)) != std::string::npos)
//     {
//         resolt_chanel.push_back(str.substr(start, end - start));
//         start = end + 1;
//     }

//     resolt_chanel.push_back(str.substr(start));
//     return resolt_chanel;
// }

// this split about #chanel of the user

// OLD CODE - Requires strict order: PASS must come before NICK/USER
// void Server::processCommand(int index, std::string &message)
// {
//     std::string command;
//     std::string argument;
//     if (split(message, ' ', command, argument))
//     {
//         ft_toupper(command);
//         if (command == "PASS" || command == "NICK" || command == "USER")
//         {
//             if (clients[index]->isRegistered() == false)
//             {
//                 if (clients[index]->isPassOk() == false)
//                 {
//                     if (check_passok(command, argument, index))
//                     {
//                         this->clients[index]->setPassOk(true);
//                         return;
//                     }
//                     sendError(this->clients[index]->get_fd(), "error clean message buffer\r\n");
//                     message = "";
//                     return;
//                 }
//             }
//             if (check_authentication(command, argument, index) == false)
//             {
//                 sendError(this->clients[index]->get_fd(), "error clean message buffer\r\n");
//                 message = "";
//                 return;
//             }
//             if (this->clients[index]->isRegistered() && this->clients[index]->isWelcomeSent() == false)
//             {
//                 sendError(this->clients[index]->get_fd(), "001 RPL_WELCOME :Welcome to the Internet Relay Network " +
//                     this->clients[index]->getNickname() + "!" +
//                     this->clients[index]->getUsername() + "@" + this->clients[index]->getIP() + "\r\n");
//                 this->clients[index]->setWelcomeSent(true);
//             }
//             return;
//         }
//         ...
//     }
// }

// NEW CODE - Flexible order for PASS/NICK/USER (RFC 1459 compliant)
void Server::processCommand(int index, std::string &message)
{
    std::string command;
    std::string argument;

    if (split(message, ' ', command, argument))
    {
        // Handle PASS, NICK, USER for registration
		ft_toupper(command);
        if (command == "PASS" || command == "NICK" || command == "USER")
        {
            // NEW: Handle PASS command first if it's PASS
            if (command == "PASS")
            {
                if (check_passok(command, argument, index))
                {
                    this->clients[index]->setPassOk(true);
                }
                else
                {
                    message = "";
                }
                return;
            }
            
            // NEW: Handle NICK and USER - they can come in any order now
            // We just need to ensure all three (PASS, NICK, USER) are complete for registration
            if (check_authentication(command, argument, index) == false)
            {
                message = "";
                return;
            }
            
            // NEW: Check if registration is now complete and send welcome message
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
        {
            this->handel_Join(command, argument, index);
        }
        else if (command == "TOPIC")
        {
            this->handel_Topic(command, argument, index);
        }
        else if (command == "INVITE")
        {
            this->handel_Invite(command, argument, index);
        
        }
        // sendError(this->clients[index]->get_fd(),"["  + argument + "]\r\n");
        //  add more commands here like JOIN, PART, PRIVMSG, etc. use cmmand and argument variables

        // youssef part : "kick , privmsg , mode"
        
        else if (command == "PRIVMSG")
            {
                this->handle_privmsg(index, argument);
            }
        else if (command == "KICK")
            {
                this->handle_kick(index, argument);
            }
        else if (command == "MODE")
            {
                handle_mode(index, argument);
            }
    }
}

void sendError(int fd, const std::string &msg)
{
    if (send(fd, msg.c_str(), msg.length(), 0) == -1)
        write(2, "Error sending data to client.\n", 30);
}

bool Server::isNicknameTaken(std::string nickname, int excludeIndex)
{
	// for (int i = 1; i < g_num_fds; i++)
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

void Server::Quit()
{
	// Close all client connections
	// for (int i = 1; i < g_num_fds; ++i)
	for (int i = 1; i < (int)this->clients.size(); ++i)
	{
		if (this->clients[i])
		{
			close(this->clients[i]->get_fd());
			delete this->clients[i];
			this->clients[i] = NULL;
		}
	}
	// Close server socket
	// close(this->server_Fd);
	// exit(0);
	
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
