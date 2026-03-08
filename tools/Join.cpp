#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

std::vector<std::string> split_chanel(const std::string &str, char delemeter)
{
    std::vector<std::string> resolt_chanel;

    int start = 0;
    int end;
    while ((end = (int)str.find(delemeter, start)) != (int)std::string::npos)
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
        sendError(this->clients[index]->get_fd(), "461 " + this->clients[index]->getNickname() + " " + command + " :Not enough parameters\r\n");
        return;
    }

    std::string chanel;
    std::string key;
    if (split(argument, ' ', chanel, key))
    {
        std::vector<std::string> all_chanel = split_chanel(chanel, ',');
        std::vector<std::string> all_key;
        if (!key.empty())
            all_key = split_chanel(key, ',');

        for (size_t i = 0; i < all_chanel.size(); i++)
        {
            std::string &chanName = all_chanel[i];

            if (chanName.empty())
            {
                sendError(this->clients[index]->get_fd(), "461 " + this->clients[index]->getNickname() + " " + chanName + " :Not enough parameters\r\n");
                continue;
            }
            if (chanName[0] != '#' && chanName[0] != '&')
            {
                sendError(this->clients[index]->get_fd(), "476 " + this->clients[index]->getNickname() + " " + chanName + " :Bad Channel Mask\r\n");
                continue;
            }
            if (chanName.size() < 2 || chanName.size() > 50)
            {
                sendError(this->clients[index]->get_fd(), "476 " + this->clients[index]->getNickname() + " " + chanName + " :Bad Channel Mask\r\n");
                continue;
            }
            if (chanName.find(' ') != std::string::npos || chanName.find(',') != std::string::npos || chanName.find('\7') != std::string::npos)
            {
                sendError(this->clients[index]->get_fd(), "476 " + this->clients[index]->getNickname() + " " + chanName + " :Bad Channel Mask\r\n");
                continue;
            }

            bool is_new_channel = !this->findChannel(chanName);
            Channel *channel = this->findOrCreateChannel(chanName);

            if (channel->hasUser(this->clients[index]))
                continue;

            if (!is_new_channel && channel->is_invite_only())
            {
                if (!channel->has_invite(this->clients[index]))
                {
                    sendError(this->clients[index]->get_fd(), ":server 473 " + this->clients[index]->getNickname() + " " + chanName + " :Cannot join channel (+i)\r\n");
                    continue;
                }

                channel->removeInvite(this->clients[index]);
            }


            if (!is_new_channel && channel->hasAkeys())
            {
                if (i >= all_key.size() || !channel->checkAkey(all_key[i]))
                {
                    sendError(this->clients[index]->get_fd(), ":server 475 " + this->clients[index]->getNickname() + " " + chanName + " :Cannot join channel (+k)\r\n");
                    continue;
                }
            }

            if (!is_new_channel && channel->get_user_limit() > 0)
            {
                int current = (int)channel->get_members().size();
                if (current >= channel->get_user_limit())
                {
                    sendError(this->clients[index]->get_fd(), ":server 471 " + this->clients[index]->getNickname() + " " + chanName + " :Cannot join channel (+l)\r\n");
                    continue;
                }
            }

            if (is_new_channel && i < all_key.size() && !all_key[i].empty())
            {
                channel->setAkey(all_key[i]);
            }

            channel->addUser(this->clients[index]);
            channel->add_member(this->clients[index]);

            if (is_new_channel || channel->get_members().size() == 1)
                channel->add_operator(this->clients[index]);

            std::string joinMsg = ":" + this->clients[index]->getNickname()
                                + "!" + this->clients[index]->getUsername()
                                + "@" + this->clients[index]->getIP()
                                + " JOIN :" + chanName + "\r\n";
            send(this->clients[index]->get_fd(), joinMsg.c_str(), joinMsg.size(), 0);


            channel->broadcast(joinMsg, this->clients[index]);

            if (!channel->get_topic().empty())
            {
                std::string topicMsg = ":server 332 " + this->clients[index]->getNickname()
                                     + " " + chanName + " :" + channel->get_topic() + "\r\n";
                send(this->clients[index]->get_fd(), topicMsg.c_str(), topicMsg.size(), 0);
            }


            std::string usersList = channel->getuserList();
            std::string namesReply = ":server 353 " + this->clients[index]->getNickname()
                                   + " = " + chanName + " :" + usersList + "\r\n";
            send(this->clients[index]->get_fd(), namesReply.c_str(), namesReply.size(), 0);

            std::string endOfNames = ":server 366 " + this->clients[index]->getNickname()
                                   + " " + chanName + " :End of /NAMES list\r\n";
            send(this->clients[index]->get_fd(), endOfNames.c_str(), endOfNames.size(), 0);
        }
    }
}
