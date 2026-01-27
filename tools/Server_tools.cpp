#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

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

// this split about #chanel of the user

std::vector<std::string> split_chanel(const std::string &str, char delemeter)
{
    std::vector<std::string> resolt_chanel;

    int start = 0;
    int end;
    while ((end = str.find(delemeter, start)) != std::string::npos)
    {
        resolt_chanel.push_back(str.substr(start, end - start));
        start = end + 1;
    }

    resolt_chanel.push_back(str.substr(start));
    return resolt_chanel;
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
                    sendError(this->clients[index]->get_fd(), "error clean message buffer\r\n"); // for debug
                    message = "";
                    return;
                }
            }
            if (check_authentication(command, argument, index) == false)
            {
                sendError(this->clients[index]->get_fd(), "error clean message buffer\r\n"); // for debug
                message = "";
                return;
            }
            if (this->clients[index]->isRegistered() && this->clients[index]->isWelcomeSent() == false)
            {
                //
                //   "Welcome to the Internet Relay Network
                //    <nick>!<user>@<host>

                // sendError(this->clients[index]->get_fd(), "pass is > [" + this->password + "]\n" + "nick is > [" + clients[index]->getNickname() + "]\n"
                // 	"user is > [" + clients[index]->getUsername() + "]\n"+ "\r\n");
                // sendError(this->clients[index]->get_fd(), "server 001 " + this->clients[index]->getNickname() + " : Welcome to the Internet Relay Network\r\n");
                sendError(this->clients[index]->get_fd(), "001 RPL_WELCOME :Welcome to the Internet Relay Network " +
                                                              this->clients[index]->getNickname() + "!" +
                                                              this->clients[index]->getUsername() + "@" + this->clients[index]->getIP() + "\r\n");
                this->clients[index]->setWelcomeSent(true);
            }
            return;
        }
        else if (this->clients[index]->isRegistered() == false)
        {

            sendError(this->clients[index]->get_fd(), "451 ERR_NOTREGISTERED : You have not registered\r\n");
            sendError(this->clients[index]->get_fd(), "error clean message buffer\r\n"); // for debug
            message = "";
            return;
        }
        else if (command == "JOIN")
        {

            if (argument.empty())
            {
                sendError(this->clients[index]->get_fd(), "461 " + command + " Not enough parameters\r\n");
                return;
            }
            // if (argument[0] != '#' && argument[0] != '&')
            // {
            // 	sendError(this->clients[index]->get_fd(), "476 " + command + " Bad Channel Mask\r\n");
            // 	return;
            // }
            // if (argument.size() < 2)
            // {
            // 	sendError(this->clients[index]->get_fd(), "476 " + command + " Channel name too short\r\n");
            // 	return;
            // }
            std::string chanel;
            std::string key;
            if (split(argument, ' ', chanel, key))
            {
                // ABOUT CHANEL
                std::vector<std::string> all_chanel = split_chanel(chanel, ',');
                std::vector<std::string> all_key;
                if (!key.empty())
                {
                    all_key = split_chanel(key, ',');
                }
                for (size_t i = 0; i < all_chanel.size(); i++)
                {
                    std::string &chanName = all_chanel[i];

                    if (chanName.empty())
                    {
                        sendError(this->clients[index]->get_fd(), "461 " + chanName + " Not enough parameters\r\n");
                        continue;
                    }
                    if (chanName[0] != '#' && chanName[0] != '&')
                    {
                        sendError(this->clients[index]->get_fd(), "476 " + chanName + " Bad Channel Mask\r\n");
                        continue;
                    }
                    if (chanName.size() < 2 || chanName.size() > 50)
                    {
                        sendError(this->clients[index]->get_fd(), "476 " + this->clients[index]->getNickname() + " " + chanName + " Bad Channel Mask\r\n");
                        continue;
                    }
                    if (chanName.find(' ') != std::string::npos || chanName.find(',') != std::string::npos || chanName.find('\7') != std::string::npos)
                    {
                        sendError(this->clients[index]->get_fd(), "476 " + chanName + " Bad Channel Mask\r\n");
                        continue;
                    }
                    Channel *channel = this->findOrCreateChannel(chanName);
                    if (channel->hasUser(this->clients[index]))
                    {
                        continue;
                    };
                    if (channel->hasAkeys())
                    {
                        if (i >= all_key.size() || !channel->checkAkey(all_key[i]))
                        {
                            sendError(this->clients[index]->get_fd(), " 475 " + chanName + " : Cannot join channel (+k)\r\n");
                            continue;
                        }
                    }
                    else if (i < all_key.size() && !all_key[i].empty())
                    {
                        channel->setAkey(all_key[i]);
                    }

                    channel->addUser(this->clients[index]);

                    std::string joinMsg = ":" + this->clients[index]->getNickname() + "!" + this->clients[index]->getUsername() + " JOIN " + chanName + "\r\n";
                    send(this->clients[index]->get_fd(), joinMsg.c_str(), joinMsg.size(), 0);

                    channel->broadcast(joinMsg, this->clients[index]);
                    std::string usersList = channel->getuserList();
                    std::string namesRoply = ":server 353 " + this->clients[index]->getNickname() + " = " + chanName + " :" + usersList + "\r\n";
                    send(this->clients[index]->get_fd(), namesRoply.c_str(), namesRoply.size(), 0);
                    std::string endOfName = ":server 366 " + this->clients[index]->getNickname() + " = " + chanName + " :End of /NAMES list\r\n";
                    send(this->clients[index]->get_fd(), endOfName.c_str(), endOfName.size(), 0);
                    // std::cout << joinMsg << std::endl;
                    // std::cout << this->clients[index] << std::endl;
                }

                // for (size_t i = 0 ; i  < all_key.size() ;i++)
                // {
                // 	std::cout << "------------> " << all_key[i] << std::endl;
                // }
            }
        }
        else if (command == "TOPIC")
        {
            if (argument.empty())
            {
                sendError(this->clients[index]->get_fd(), "461 " + clients[index]->getNickname() + " " + command + " Not enough parameters\r\n");
                return;
            }
            std::string channel;
            std::string opject;
            // std::string new_topec;
            bool has_topeck = split(argument, ' ', channel, opject);

            if (!this->findChannel(channel))
            {
                sendError(this->clients[index]->get_fd(), "403 " + clients[index]->getNickname() + " " + channel + " :No such channel\r\n");
                return;
            }
            if (!this->_channels[channel]->hasUser(clients[index]))
            {
                sendError(this->clients[index]->get_fd(), "442 " + clients[index]->getNickname() + " " + channel + " :You're not on that channel\r\n");
                return;
            }
            if (!has_topeck || opject.empty())
            {
                if (this->_channels[channel]->get_topic().empty())
                {
                    std::string reply = ":server 331 " + this->clients[index]->getNickname() + " " + channel + " :No topic is set\r\n";
                    send(this->clients[index]->get_fd(), reply.c_str(), reply.size(), 0);
                }
                else
                {
                    std::string reply = ":server 332 " + this->clients[index]->getNickname() + " " + channel + " :" + this->_channels[channel]->get_topic() + "\r\n";
                    send(this->clients[index]->get_fd(), reply.c_str(), reply.size(), 0);
                }
                return;
            }

            if (this->_channels[channel]->is_topic_protected() && !this->_channels[channel]->is_operator(this->clients[index]))
            {
                sendError(this->clients[index]->get_fd(), "482 "+ this->clients[index]->getNickname() + " " + channel + " :You're not channel operator\r\n");
                return;
            }
            std::string new_topec = opject;
            if (!new_topec.empty() && new_topec[0] == ':')
                new_topec.erase(0, 1);
            this->_channels[channel]->set_topic(new_topec);
            this->_channels[channel]->broadcast(":" + clients[index]->get_prefix() + " TOPIC " + channel + " :" + new_topec + "\r\n", NULL);
        }
        else if (command == "INVITE")
        {
            std::string nickname;
            std::string channel;
            if (split(argument, ' ', nickname, channel))
            {
                if (nickname.empty() || channel.empty())
                {
                    sendError(this->clients[index]->get_fd(), "461 " + clients[index]->getNickname() + " " + channel + " :Not enough parameters\r\n");
                    return;
                }
                if (!this->findChannel(channel))
                {
                    sendError(this->clients[index]->get_fd(), "403 " + channel + " :No such channel\r\n");
                    return;
                }
                if (!this->_channels[channel]->hasUser(clients[index]))
                {
                    sendError(this->clients[index]->get_fd(), "442 " + clients[index]->getNickname() + " " + channel + " :You're not on that channel\r\n");
                    return;
                }
                if (_channels[channel]->hasUser(this->findClient(nickname)))
                {
                    sendError(this->clients[index]->get_fd(), "443 " + clients[index]->getNickname() + " " + channel + "  :is already on channel\r\n");
                    return;
                }
                Client *chan = this->findClient(nickname);
                if (!chan)
                {
                    sendError(this->clients[index]->get_fd(), "401 " + nickname + " :No such nick/channel\r\n");
                    return;
                }
                if (_channels[channel]->hasUser(chan))
                {
                    sendError(this->clients[index]->get_fd(), "443 " + channel + " :is already on channel\r\n");
                    return;
                }
                if (this->_channels[channel]->is_invite_only() && !this->_channels[channel]->is_operator(this->clients[index]))
                {
                    sendError(this->clients[index]->get_fd(), "482 " + channel + " :You're not channel operator\r\n");
                    return;
                }
                _channels[channel]->addUserInvite(chan);
                std::string reply = ":" + clients[index]->get_prefix() + " INVITE " + nickname + " :" + channel + "\r\n";
                send(chan->get_fd(), reply.c_str(), reply.size(), 0);
                std::string confirm = ":server 341 " + clients[index]->getNickname() + " " + nickname + " " + channel + "\r\n";
                send(clients[index]->get_fd(), confirm.c_str(), confirm.size(), 0);
            }
        }
        // sendError(this->clients[index]->get_fd(),"["  + argument + "]\r\n");
        //  add more commands here like JOIN, PART, PRIVMSG, etc. use cmmand and argument variables

        // youssef part : "kick , privmsg , mode"
        else if (command == "PRIVMSG" || command == "KICK" || command == "MODE")
        {
            if (command == "PRIVMSG")
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
}

void sendError(int fd, const std::string &msg)
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
            return true; // Found a match in another client!
        }
    }
    return false; // Not taken
}