#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

void Server::handel_Invite(std::string &command, std::string &argument, int index)
{
    (void)command;

    std::string nickname;
    std::string channel_name;

    if (!split(argument, ' ', nickname, channel_name) || nickname.empty() || channel_name.empty())
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 461 " + clients[index]->getNickname() +
                  " INVITE :Not enough parameters\r\n");
        return;
    }

    // Channel existe ?
    if (!this->findChannel(channel_name))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 403 " + clients[index]->getNickname() +
                  " " + channel_name + " :No such channel\r\n");
        return;
    }

    Channel *chan = this->_channels[channel_name];

    // L'invitant est-il dans le channel ?
    if (!chan->has_member(clients[index]))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 442 " + clients[index]->getNickname() +
                  " " + channel_name + " :You're not on that channel\r\n");
        return;
    }

    // Si mode +i, seul un opérateur peut inviter
    if (chan->is_invite_only() && !chan->is_operator(this->clients[index]))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 482 " + clients[index]->getNickname() +
                  " " + channel_name + " :You're not channel operator\r\n");
        return;
    }

    // ✅ CORRIGÉ : utilise findClient qui parcourt le vecteur clients
    Client *target = this->findClient(nickname);
    if (!target)
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 401 " + clients[index]->getNickname() +
                  " " + nickname + " :No such nick/channel\r\n");
        return;
    }

    // Déjà dans le channel ?
    if (chan->has_member(target))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 443 " + clients[index]->getNickname() +
                  " " + nickname + " " + channel_name + " :is already on channel\r\n");
        return;
    }

    // Ajouter à la liste des invités
    chan->addUserInvite(target);

    // Notifier la cible
    std::string invite_msg = ":" + clients[index]->get_prefix() +
                             " INVITE " + nickname + " :" + channel_name + "\r\n";
    send(target->get_fd(), invite_msg.c_str(), invite_msg.size(), 0);

    // Confirmer à l'invitant
    std::string confirm = ":server 341 " + clients[index]->getNickname() +
                          " " + nickname + " " + channel_name + "\r\n";
    send(clients[index]->get_fd(), confirm.c_str(), confirm.size(), 0);
}
