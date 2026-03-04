// #include "../includes/Commands.hpp"

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

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

void Server::handel_Join(std::string &command, std::string &argument, int index)
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