#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <vector>

static std::vector<std::string> split_by(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    int start = 0;
    int end;
    while ((end = (int)str.find(delimiter, start)) != (int)std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}

void Server::handel_Join(std::string &command, std::string &argument, int index)
{
    if (argument.empty())
    {
        sendError(this->clients[index]->get_fd(),
                  "461 " + command + " :Not enough parameters\r\n");
        return;
    }

    std::string chanel_part;
    std::string key_part;
    split(argument, ' ', chanel_part, key_part);

    std::vector<std::string> all_chanel = split_by(chanel_part, ',');
    std::vector<std::string> all_key;
    if (!key_part.empty())
        all_key = split_by(key_part, ',');

    for (size_t i = 0; i < all_chanel.size(); i++)
    {
        std::string &chanName = all_chanel[i];

        if (chanName.empty())
        {
            sendError(this->clients[index]->get_fd(),
                      "461 JOIN :Not enough parameters\r\n");
            continue;
        }
        if (chanName[0] != '#' && chanName[0] != '&')
        {
            sendError(this->clients[index]->get_fd(),
                      "476 " + chanName + " :Bad Channel Mask\r\n");
            continue;
        }
        if (chanName.size() < 2 || chanName.size() > 50)
        {
            sendError(this->clients[index]->get_fd(),
                      "476 " + chanName + " :Bad Channel Mask\r\n");
            continue;
        }
        if (chanName.find(' ') != std::string::npos ||
            chanName.find(',') != std::string::npos ||
            chanName.find('\7') != std::string::npos)
        {
            sendError(this->clients[index]->get_fd(),
                      "476 " + chanName + " :Bad Channel Mask\r\n");
            continue;
        }

        Channel *channel = this->findOrCreateChannel(chanName);

        // Déjà dans le channel
        if (channel->has_member(this->clients[index]))
            continue;

        // Vérification de la limite d'utilisateurs (+l)
        if (channel->get_user_limit() > 0 &&
            (int)channel->get_members().size() >= channel->get_user_limit())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 471 " + this->clients[index]->getNickname() +
                      " " + chanName + " :Cannot join channel (+l)\r\n");
            continue;
        }

        // Vérification du mode invite-only (+i)
        if (channel->is_invite_only() && !channel->isInvited(this->clients[index]))
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 473 " + this->clients[index]->getNickname() +
                      " " + chanName + " :Cannot join channel (+i)\r\n");
            continue;
        }

        // Vérification de la clé (+k)
        if (channel->hasAkeys())
        {
            if (i >= all_key.size() || !channel->checkAkey(all_key[i]))
            {
                sendError(this->clients[index]->get_fd(),
                          ":server 475 " + this->clients[index]->getNickname() +
                          " " + chanName + " :Cannot join channel (+k)\r\n");
                continue;
            }
        }
        else if (i < all_key.size() && !all_key[i].empty())
        {
            channel->setAkey(all_key[i]);
        }

        // Ajouter le membre (addUser synchronise _members et gère le premier op)
        channel->addUser(this->clients[index]);

        // Envoyer JOIN
        std::string joinMsg = ":" + this->clients[index]->getNickname() + "!" +
                              this->clients[index]->getUsername() + "@" +
                              this->clients[index]->getIP() +
                              " JOIN " + chanName + "\r\n";
        send(this->clients[index]->get_fd(), joinMsg.c_str(), joinMsg.size(), 0);
        channel->broadcast(joinMsg, this->clients[index]);

        // Envoyer le topic si défini
        if (!channel->get_topic().empty())
        {
            std::string topicReply = ":server 332 " +
                                     this->clients[index]->getNickname() +
                                     " " + chanName + " :" +
                                     channel->get_topic() + "\r\n";
            send(this->clients[index]->get_fd(), topicReply.c_str(), topicReply.size(), 0);
        }

        // Envoyer la liste des membres (NAMES)
        std::string usersList = channel->getuserList();
        std::string namesReply = ":server 353 " +
                                  this->clients[index]->getNickname() +
                                  " = " + chanName + " :" + usersList + "\r\n";
        send(this->clients[index]->get_fd(), namesReply.c_str(), namesReply.size(), 0);

        std::string endOfNames = ":server 366 " +
                                  this->clients[index]->getNickname() +
                                  " " + chanName + " :End of /NAMES list\r\n";
        send(this->clients[index]->get_fd(), endOfNames.c_str(), endOfNames.size(), 0);
    }
}
