#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

void Server::handel_Topic(std::string &command, std::string &argument, int index)
{
    if (argument.empty())
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 461 " + clients[index]->getNickname() +
                  " TOPIC :Not enough parameters\r\n");
        return;
    }

    std::string channel_name;
    std::string topic_arg;
    bool        has_topic_arg = split(argument, ' ', channel_name, topic_arg);

    // Channel existe ?
    if (!this->findChannel(channel_name))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 403 " + clients[index]->getNickname() +
                  " " + channel_name + " :No such channel\r\n");
        return;
    }

    Channel *chan = this->_channels[channel_name];

    // L'utilisateur est-il dans le channel ?
    if (!chan->has_member(clients[index]))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 442 " + clients[index]->getNickname() +
                  " " + channel_name + " :You're not on that channel\r\n");
        return;
    }

    // Lecture du topic (pas d'argument fourni)
    if (!has_topic_arg || topic_arg.empty())
    {
        if (chan->get_topic().empty())
        {
            std::string reply = ":server 331 " +
                                this->clients[index]->getNickname() +
                                " " + channel_name + " :No topic is set\r\n";
            send(this->clients[index]->get_fd(), reply.c_str(), reply.size(), 0);
        }
        else
        {
            std::string reply = ":server 332 " +
                                this->clients[index]->getNickname() +
                                " " + channel_name + " :" +
                                chan->get_topic() + "\r\n";
            send(this->clients[index]->get_fd(), reply.c_str(), reply.size(), 0);
        }
        return;
    }

    // Modification du topic : vérifier +t
    if (chan->is_topic_protected() && !chan->is_operator(this->clients[index]))
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 482 " + this->clients[index]->getNickname() +
                  " " + channel_name + " :You're not channel operator\r\n");
        return;
    }

    // Retirer le ':' initial si présent
    std::string new_topic = topic_arg;
    if (!new_topic.empty() && new_topic[0] == ':')
        new_topic.erase(0, 1);

    chan->set_topic(new_topic);

    // Notifier tous les membres
    std::string topic_msg = ":" + clients[index]->get_prefix() +
                            " TOPIC " + channel_name + " :" + new_topic + "\r\n";
    chan->broadcast(topic_msg, NULL);

    (void)command;
}
