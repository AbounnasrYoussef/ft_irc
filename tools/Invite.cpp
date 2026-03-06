#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

void Server::handel_Invite(std::string &command, std::string &argument, int index)
{
    (void)command;
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