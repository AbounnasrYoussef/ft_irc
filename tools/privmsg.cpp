#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────

void trim(std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        s = "";
    else
        s = s.substr(start, end - start + 1);
}

static std::string format_privmsg(Client *sender, const std::string &target,
                                   const std::string &message)
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

static bool is_channel_target(const std::string &target)
{
    return (!target.empty() && (target[0] == '#' || target[0] == '&'));
}

// ─────────────────────────────────────────────
//  get_client_by_nickname  (utilisé aussi par KICK)
// ─────────────────────────────────────────────

Client *Server::get_client_by_nickname(const std::string &nickname)
{
    for (size_t i = 1; i < this->clients.size(); i++)
    {
        if (this->clients[i] && this->clients[i]->getNickname() == nickname)
            return this->clients[i];
    }
    return NULL;
}

// ─────────────────────────────────────────────
//  Parsing PRIVMSG
// ─────────────────────────────────────────────

ParsedMessage parse_arguments(const std::string &argument)
{
    ParsedMessage result;
    result.valid      = false;
    result.error_code = 0;

    // Format attendu : "<target> :<message>"
    size_t colon_pos = argument.find(':');

    if (colon_pos == std::string::npos)
    {
        // Pas de ':' → pas de texte
        result.error_code = 412; // ERR_NOTEXTTOSEND
        return result;
    }

    result.target = argument.substr(0, colon_pos);
    trim(result.target);

    if (result.target.empty())
    {
        result.error_code = 411; // ERR_NORECIPIENT
        return result;
    }

    result.message = argument.substr(colon_pos + 1);

    if (result.message.empty())
    {
        result.error_code = 412; // ERR_NOTEXTTOSEND
        return result;
    }

    result.valid = true;
    return result;
}

// ─────────────────────────────────────────────
//  Envoi vers un utilisateur
// ─────────────────────────────────────────────

static void send_to_user(Server *server, Client *sender,
                          const std::string &target_nick,
                          const std::string &message)
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

// ─────────────────────────────────────────────
//  Envoi vers un channel
// ─────────────────────────────────────────────

static void send_to_channel(Server *server, Client *sender,
                             const std::string &chan_name,
                             const std::string &message)
{
    Channel *channel = server->get_channel(chan_name);
    if (!channel)
    {
        std::string error = ":server 403 ";
        error += sender->getNickname();
        error += " ";
        error += chan_name;
        error += " :No such channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }

    // Vérifier que l'expéditeur est bien dans le channel
    if (!channel->has_member(sender))
    {
        std::string error = ":server 404 ";
        error += sender->getNickname();
        error += " ";
        error += chan_name;
        error += " :Cannot send to channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }

    std::string formatted = format_privmsg(sender, chan_name, message);
    // Envoyer à tous les membres SAUF l'expéditeur
    std::vector<Client *> members = channel->get_members();
    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i] != sender)
            send(members[i]->get_fd(), formatted.c_str(), formatted.length(), 0);
    }
}

// ─────────────────────────────────────────────
//  handle_privmsg  — point d'entrée
// ─────────────────────────────────────────────

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
        else
        {
            error = ":server 412 ";
            error += sender->getNickname();
            error += " :No text to send\r\n";
        }
        sendError(sender->get_fd(), error);
        return;
    }

    // ✅ Logique correcte : channel → send_to_channel, sinon → send_to_user
    if (is_channel_target(parsed.target))
        send_to_channel(this, sender, parsed.target, parsed.message);
    else
        send_to_user(this, sender, parsed.target, parsed.message);
}
