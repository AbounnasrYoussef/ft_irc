#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

void Server::handel_Topic(std::string &command, std::string &argument, int index)
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
        sendError(this->clients[index]->get_fd(), "482 " + this->clients[index]->getNickname() + " " + channel + " :You're not channel operator\r\n");
        return;
    }
    std::string new_topec = opject;
    if (!new_topec.empty() && new_topec[0] == ':')
        new_topec.erase(0, 1);
    this->_channels[channel]->set_topic(new_topec);
    this->_channels[channel]->broadcast(":" + clients[index]->get_prefix() + " TOPIC " + channel + " :" + new_topec + "\r\n", NULL);
}