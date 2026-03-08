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
        sendError(this->clients[index]->get_fd(), ":server 461 " + clients[index]->getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }


    if (!this->findChannel(channel))
    {
        sendError(this->clients[index]->get_fd(), ":server 403 " + clients[index]->getNickname() + " " + channel + " :No such channel\r\n");
        return;
    }


    if (!this->_channels[channel]->hasUser(clients[index]))
    {
        sendError(this->clients[index]->get_fd(), ":server 442 " + clients[index]->getNickname() + " " + channel + " :You're not on that channel\r\n");
        return;
    }


    if (this->_channels[channel]->is_invite_only() && !this->_channels[channel]->is_operator(this->clients[index]))
    {
        sendError(this->clients[index]->get_fd(), ":server 482 " + clients[index]->getNickname() + " " + channel + " :You're not channel operator\r\n");
        return;
    }


    Client *target = this->findClient(nickname);
    if (!target)
    {
        sendError(this->clients[index]->get_fd(), ":server 401 " + clients[index]->getNickname() + " " + nickname + " :No such nick/channel\r\n");
        return;
    }


    if (_channels[channel]->hasUser(target))
    {
        sendError(this->clients[index]->get_fd(), ":server 443 " + clients[index]->getNickname() + " " + nickname + " " + channel + " :is already on channel\r\n");
        return;
    }

    
    _channels[channel]->addUserInvite(target);

    std::string reply = ":" + clients[index]->get_prefix() + " INVITE " + nickname + " :" + channel + "\r\n";
    send(target->get_fd(), reply.c_str(), reply.size(), 0);

    std::string confirm = ":server 341 " + clients[index]->getNickname() + " " + nickname + " " + channel + "\r\n";
    send(clients[index]->get_fd(), confirm.c_str(), confirm.size(), 0);
}
}
