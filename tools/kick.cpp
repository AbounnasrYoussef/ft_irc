#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"

// ─────────────────────────────────────────────
//  Parsing des arguments KICK
//  Format : KICK <channel> <target> [:<reason>]
// ─────────────────────────────────────────────

ParsedKick parse_kick_arguments(const std::string &argument)
{
    ParsedKick result;
    result.valid      = false;
    result.error_code = 0;
    result.reason     = "";

    std::string params;

    // Séparer la raison (après ':')
    size_t colon_pos = argument.find(':');
    if (colon_pos != std::string::npos)
    {
        result.reason = argument.substr(colon_pos + 1);
        params        = argument.substr(0, colon_pos);
    }
    else
    {
        params = argument;
    }

    trim(params);

    // Séparer channel et cible
    size_t space_pos = params.find(' ');
    if (space_pos == std::string::npos)
    {
        result.error_code = 461;
        result.error_msg  = "KICK :Not enough parameters";
        return result;
    }

    result.channel     = params.substr(0, space_pos);
    result.target_nick = params.substr(space_pos + 1);

    trim(result.channel);
    trim(result.target_nick);

    if (result.channel.empty() || result.target_nick.empty())
    {
        result.error_code = 461;
        result.error_msg  = "KICK :Not enough parameters";
        return result;
    }

    result.valid = true;
    return result;
}

// ─────────────────────────────────────────────
//  Formatage du message KICK
// ─────────────────────────────────────────────

static std::string format_kick(Client *kicker, const std::string &channel,
                                const std::string &target,
                                const std::string &reason)
{
    std::string result = ":";
    result += kicker->getNickname();
    result += "!";
    result += kicker->getUsername();
    result += "@";
    result += kicker->getIP();
    result += " KICK ";
    result += channel;
    result += " ";
    result += target;
    if (!reason.empty())
    {
        result += " :";
        result += reason;
    }
    result += "\r\n";
    return result;
}

// ─────────────────────────────────────────────
//  handle_kick — point d'entrée
// ─────────────────────────────────────────────

void Server::handle_kick(int kicker_index, const std::string &argument)
{
    Client *kicker = this->clients[kicker_index];

    // 1. Parser
    ParsedKick parsed = parse_kick_arguments(argument);
    if (!parsed.valid)
    {
        std::string error = ":server 461 ";
        error += kicker->getNickname();
        error += " KICK :Not enough parameters\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 2. Vérifier que le channel existe
    Channel *channel = this->get_channel(parsed.channel);
    if (!channel)
    {
        std::string error = ":server 403 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :No such channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 3. Vérifier que le kicker est dans le channel
    if (!channel->has_member(kicker))
    {
        std::string error = ":server 442 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not on that channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 4. Vérifier que le kicker est opérateur
    if (!channel->is_operator(kicker))
    {
        std::string error = ":server 482 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not channel operator\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 5. Trouver la cible
    Client *target = this->get_client_by_nickname(parsed.target_nick);
    if (!target || !channel->has_member(target))
    {
        std::string error = ":server 441 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.target_nick;
        error += " ";
        error += parsed.channel;
        error += " :They aren't on that channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 6. Construire et diffuser le message KICK à tous (y compris la cible)
    std::string kick_msg = format_kick(kicker, parsed.channel,
                                        parsed.target_nick, parsed.reason);
    std::vector<Client *> members = channel->get_members();
    for (size_t i = 0; i < members.size(); i++)
        send(members[i]->get_fd(), kick_msg.c_str(), kick_msg.length(), 0);

    // 7. Retirer la cible du channel
    channel->remove_member(target);

    // 8. Supprimer le channel s'il est vide
    if (channel->is_empty())
        this->delete_channel(channel);
}
