#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Commands.hpp"
#include <algorithm>
#include <vector>

bool Server::check_passok(std::string command, std::string argument, int index)
{
    if (command == "PASS")
    {
        if (this->clients[index]->isRegistered())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 462 " + clients[index]->getNickname() +
                      " :You may not reregister\r\n");
            return false;
        }
        if (this->clients[index]->isPassOk())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 462 " + clients[index]->getNickname() +
                      " :You may not reregister\r\n");
            return false;
        }
        if (argument.empty())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 461 PASS :Not enough parameters\r\n");
            return false;
        }
        if (argument != this->password)
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 464 :Password incorrect\r\n");
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
            sendError(this->clients[index]->get_fd(),
                      ":server 451 :You have not registered\r\n");
            return false;
        }
        if (argument.empty())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 431 :No nickname given\r\n");
            return false;
        }
        if (isNicknameTaken(argument, index))
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 433 " + argument + " :Nickname is already in use\r\n");
            return false;
        }
        if (!pars_nick(argument))
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 432 " + argument + " :Erroneous nickname\r\n");
            return false;
        }
        this->clients[index]->setNickname(argument);
        return true;
    }
    else if (command == "USER")
    {
        if (this->clients[index]->isRegistered())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 462 " + clients[index]->getNickname() +
                      " :You may not reregister\r\n");
            return false;
        }
        if (!this->clients[index]->isPassOk())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 451 :You have not registered\r\n");
            return false;
        }
        if (argument.empty())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 461 USER :Not enough parameters\r\n");
            return false;
        }
        if (!user_parsing(argument, this->clients[index]))
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 461 USER :Not enough parameters\r\n");
            return false;
        }
        return true;
    }
    return true;
}

void Server::processCommand(int index, std::string &message)
{
    std::string command;
    std::string argument;

    if (!split(message, ' ', command, argument))
        return;

    ft_toupper(command);

    // ── Commandes de registration ──────────────────────────────────────
    if (command == "PASS" || command == "NICK" || command == "USER")
    {
        if (!clients[index]->isRegistered())
        {
            if (!clients[index]->isPassOk())
            {
                if (check_passok(command, argument, index))
                {
                    this->clients[index]->setPassOk(true);
                    return;
                }
                message = "";
                return;
            }
        }
        if (!check_authentication(command, argument, index))
        {
            message = "";
            return;
        }
        // Envoyer le message de bienvenue dès que l'enregistrement est complet
        if (this->clients[index]->isRegistered() &&
            !this->clients[index]->isWelcomeSent())
        {
            sendError(this->clients[index]->get_fd(),
                      ":server 001 " + this->clients[index]->getNickname() +
                      " :Welcome to the Internet Relay Network " +
                      this->clients[index]->getNickname() + "!" +
                      this->clients[index]->getUsername() + "@" +
                      this->clients[index]->getIP() + "\r\n");
            this->clients[index]->setWelcomeSent(true);
        }
        return;
    }

    // ── Commandes nécessitant une registration complète ────────────────
    if (!this->clients[index]->isRegistered())
    {
        sendError(this->clients[index]->get_fd(),
                  ":server 451 :You have not registered\r\n");
        message = "";
        return;
    }

    if (command == "JOIN")
        this->handel_Join(command, argument, index);
    else if (command == "TOPIC")
        this->handel_Topic(command, argument, index);
    else if (command == "INVITE")
        this->handel_Invite(command, argument, index);
    else if (command == "PRIVMSG")
        this->handle_privmsg(index, argument);
    else if (command == "KICK")
        this->handle_kick(index, argument);
    else if (command == "MODE")
        this->handle_mode(index, argument);
    else if (command == "QUIT")
    {
        // Déconnexion propre
        std::string quit_msg = ":" + clients[index]->get_prefix() +
                               " QUIT :Goodbye\r\n";
        // Notifier les channels
        removeClient(this->_fds, this->clients, index);
    }
    // Commande inconnue → ignorer silencieusement (ou envoyer 421)
}

void sendError(int fd, const std::string &msg)
{
    if (send(fd, msg.c_str(), msg.length(), 0) == -1)
        write(2, "Error sending data to client.\n", 30);
}

bool Server::isNicknameTaken(std::string nickname, int excludeIndex)
{
    for (int i = 1; i < (int)this->_fds.size(); i++)
    {
        if (this->clients[i] && i != excludeIndex &&
            this->clients[i]->getNickname() == nickname)
            return true;
    }
    return false;
}

int Server::getServerFd()
{
    return this->server_Fd;
}

void Server::Quit()
{
    for (int i = 1; i < (int)this->clients.size(); ++i)
    {
        if (this->clients[i])
        {
            close(this->clients[i]->get_fd());
            delete this->clients[i];
            this->clients[i] = NULL;
        }
    }
}

void ft_toupper(std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        str[i] = std::toupper(str[i]);
}
