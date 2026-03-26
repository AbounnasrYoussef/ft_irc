#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"
#include <sstream>
#include <cstdlib>

static void send_channel_modes(Client *client, Channel *channel)
{
    std::string modes = "+";

    if (channel->is_invite_only())    modes += "i";
    if (channel->is_topic_protected()) modes += "t";
    if (channel->is_moderated())      modes += "m";
    if (channel->is_no_external())    modes += "n";
    if (channel->hasAkeys())          modes += "k";
    if (channel->get_user_limit() > 0) modes += "l";

    std::string response = ":server 324 ";
    response += client->getNickname();
    response += " ";
    response += channel->get_name();
    response += " ";
    response += modes;
    response += "\r\n";

    send(client->get_fd(), response.c_str(), response.length(), 0);
}

static ParsedMode parse_mode_arguments(const std::string &argument)
{
    ParsedMode result;
    result.valid = false;

    size_t space_pos = argument.find(' ');

    if (space_pos == std::string::npos)
    {
        result.channel = argument;
        trim(result.channel);
        result.valid = true;
        return result;
    }

    result.channel = argument.substr(0, space_pos);
    trim(result.channel);

    std::string rest = argument.substr(space_pos + 1);
    trim(rest);

    if (rest.empty())
    {
        result.valid = true;
        return result;
    }
    size_t next_space = rest.find(' ');
    std::string mode_string;
    std::string params_string;

    if (next_space == std::string::npos)
    {
        mode_string = rest;
        params_string = "";
    }
    else
    {
        mode_string = rest.substr(0, next_space);
        params_string = rest.substr(next_space + 1);
        trim(params_string);
    }
    std::vector<std::string> params;
    {
        std::istringstream iss(params_string);
        std::string token;
        while (iss >> token)
            params.push_back(token);
    }
    bool adding = true;
    size_t param_idx = 0;

    for (size_t i = 0; i < mode_string.length(); i++)
    {
        char c = mode_string[i];

        if (c == '+')
        {
            adding = true;
        }
        else if (c == '-')
        {
            adding = false;
        }
        else
        {
            ModeChange change;
            change.mode = c;
            change.adding = adding;
            change.param = "";

            if ((c == 'o' || (adding && (c == 'k' || c == 'l'))))
            {
                if (param_idx < params.size())
                {
                    change.param = params[param_idx];
                    param_idx++;
                }
            }

            result.changes.push_back(change);
        }
    }

    result.valid = true;
    return result;
}

void Server::handle_mode(int setter_index, const std::string &argument)
{
    Client *setter = this->clients[setter_index];

    ParsedMode parsed = parse_mode_arguments(argument);

    if (!parsed.valid)
    {
        std::string error = ":server 461 ";
        error += setter->getNickname();
        error += " MODE :Not enough parameters\r\n";
        sendError(setter->get_fd(), error);
        return;
    }

    Channel *channel = this->get_channel(parsed.channel);
    if (!channel)
    {
        std::string error = ":server 403 ";
        error += setter->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :No such channel\r\n";
        sendError(setter->get_fd(), error);
        return;
    }

    if (parsed.changes.empty())
    {
        send_channel_modes(setter, channel);
        return;
    }

    if (!channel->hasUser(setter))
    {
        std::string error = ":server 442 ";
        error += setter->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not on that channel\r\n";
        sendError(setter->get_fd(), error);
        return;
    }

    if (!channel->is_operator(setter))
    {
        std::string error = ":server 482 ";
        error += setter->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not channel operator\r\n";
        sendError(setter->get_fd(), error);
        return;
    }

    std::string applied_modes = "";
    std::string applied_params = "";

    for (size_t i = 0; i < parsed.changes.size(); i++)
    {
        ModeChange &change = parsed.changes[i];

        if (change.mode == 'i')
        {
            channel->set_invite_only(change.adding);
            applied_modes += change.adding ? "+i" : "-i";
        }
        else if (change.mode == 't')
        {
            channel->set_topic_protected(change.adding);
            applied_modes += change.adding ? "+t" : "-t";
        }
        else if (change.mode == 'm')
        {
            channel->set_moderated(change.adding);
            applied_modes += change.adding ? "+m" : "-m";
        }
        else if (change.mode == 'n')
        {
            channel->set_no_external(change.adding);
            applied_modes += change.adding ? "+n" : "-n";
        }
        else if (change.mode == 'o')
        {
            if (change.param.empty())
            {
                std::string error = ":server 461 ";
                error += setter->getNickname();
                error += " MODE :Not enough parameters\r\n";
                sendError(setter->get_fd(), error);
                continue;
            }

            Client *target = this->get_client_by_nickname(change.param);
            if (!target || !channel->hasUser(target))
            {
                std::string error = ":server 441 ";
                error += setter->getNickname();
                error += " ";
                error += change.param;
                error += " ";
                error += parsed.channel;
                error += " :They aren't on that channel\r\n";
                sendError(setter->get_fd(), error);
                continue;
            }

            if (change.adding)
                channel->add_operator(target);
            else
                channel->remove_operator(target);

            applied_modes += change.adding ? "+o" : "-o";
            applied_params += " ";
            applied_params += change.param;
        }
        else if (change.mode == 'k')
        {
            if (change.adding)
            {
                if (change.param.empty())
                {
                    std::string error = ":server 461 ";
                    error += setter->getNickname();
                    error += " MODE :Not enough parameters\r\n";
                    sendError(setter->get_fd(), error);
                    continue;
                }
                channel->setAkey(change.param);
                applied_modes += "+k";
                applied_params += " ";
                applied_params += change.param;
            }
            else
            {
                channel->setAkey("");
                applied_modes += "-k";
            }
        }
        else if (change.mode == 'l')
        {
            if (change.adding)
            {
                if (change.param.empty())
                {
                    std::string error = ":server 461 ";
                    error += setter->getNickname();
                    error += " MODE :Not enough parameters\r\n";
                    sendError(setter->get_fd(), error);
                    continue;
                }
                int limit = std::atoi(change.param.c_str());
                if (limit <= 0)
                {
                    std::string error = ":server 461 ";
                    error += setter->getNickname();
                    error += " MODE :Invalid limit parameter\r\n";
                    sendError(setter->get_fd(), error);
                    continue;
                }
                channel->set_user_limit(limit);
                applied_modes += "+l";
                applied_params += " ";
                applied_params += change.param;
            }
            else
            {
                channel->set_user_limit(0);
                applied_modes += "-l";
            }
        }
        else
        {
            std::string error = ":server 472 ";
            error += setter->getNickname();
            error += " ";
            error += change.mode;
            error += " :is unknown mode char to me\r\n";
            sendError(setter->get_fd(), error);
        }
    }

    if (!applied_modes.empty())
    {
        std::string mode_msg = ":";
        mode_msg += setter->getNickname();
        mode_msg += "!";
        mode_msg += setter->getUsername();
        mode_msg += "@";
        mode_msg += setter->getIP();
        mode_msg += " MODE ";
        mode_msg += parsed.channel;
        mode_msg += " ";
        mode_msg += applied_modes;
        mode_msg += applied_params;
        mode_msg += "\r\n";
        channel->broadcast(mode_msg, NULL);
    }
}
