#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"

std::string format_privmsg(Client *sender, const std::string &target, const std::string &message)
{
    std::string result = ":";
    result += sender->getNickname();
    result += "!";
    result += sender->getUsername();
    result += "@";
    result += sender->getIP();
    result += " PRIVMSG ";
    result += target;
    result += " :";
    result += message;
    result += "\r\n";
    return result;
}

bool is_channel(const std::string &target)
{
    return (!target.empty() && (target[0] == '#' || target[0] == '&'));
}

Client *Server::get_client_by_nickname(const std::string &nickname)
{
    for (int i = 1; i < (int)this->_fds.size(); i++)
    {
        if (this->clients[i] && this->clients[i]->getNickname() == nickname)
            return this->clients[i];
    }
    return NULL;
}

void trim(std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");

    if (start == std::string::npos)
        s = "";
    else
        s = s.substr(start, end - start + 1);
}

ParsedMessage parse_arguments(const std::string &argument)
{
    ParsedMessage result;
    result.valid = false;
    result.error_code = 0;

    if (argument.empty())
    {
        result.error_code = 411;
        return result;
    }
    size_t space_pos = argument.find(' ');

    if (space_pos == std::string::npos)
    {
        result.error_code = 412;
        return result;
    }

    result.target = argument.substr(0, space_pos);
    trim(result.target);

    if (result.target.empty())
    {
        result.error_code = 411;
        return result;
    }

    std::string rest = argument.substr(space_pos + 1);

    if (rest.empty())
    {
        result.error_code = 412;
        return result;
    }
    if (rest[0] == ':')
        result.message = rest.substr(1);
    else
        result.message = rest;

    if (result.message.empty())
    {
        result.error_code = 412;
        return result;
    }

    result.valid = true;
    return result;
}

static void send_to_channel(Server *server, Client *sender, const std::string &target_name, const std::string &message)
{
    Channel *channel = server->get_channel(target_name);

    if (!channel)
    {
        std::string error = ":server 403 ";
        error += sender->getNickname();
        error += " ";
        error += target_name;
        error += " :No such channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }
    if (!channel->hasUser(sender))
    {
        std::string error = ":server 404 ";
        error += sender->getNickname();
        error += " ";
        error += target_name;
        error += " :Cannot send to channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }
    std::string formatted = format_privmsg(sender, target_name, message);
    channel->broadcast(formatted, sender);
}

static void send_to_user(Server *server, Client *sender, const std::string &target_nick, const std::string &message)
{
    Client *target = server->get_client_by_nickname(target_nick);

    if (!target)
    {
        std::string error = ":server 401 ";
        error += sender->getNickname();
        error += " ";
        error += target_nick;
        error += " :No such nick/channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }

    std::string formatted = format_privmsg(sender, target_nick, message);
    send(target->get_fd(), formatted.c_str(), formatted.length(), 0);
}

void Server::handle_privmsg(int sender_index, const std::string &argument)
{
    Client *sender = this->clients[sender_index];

    ParsedMessage parsed = parse_arguments(argument);

    if (!parsed.valid)
    {
        std::string error;
        if (parsed.error_code == 411)
        {
            error = ":server 411 ";
            error += sender->getNickname();
            error += " :No recipient given (PRIVMSG)\r\n";
        }
        else if (parsed.error_code == 412)
        {
            error = ":server 412 ";
            error += sender->getNickname();
            error += " :No text to send\r\n";
        }
        sendError(sender->get_fd(), error);
        return;
    }

    if (is_channel(parsed.target))
        send_to_channel(this, sender, parsed.target, parsed.message);
    else
        send_to_user(this, sender, parsed.target, parsed.message);
}
