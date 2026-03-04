#include "../includes/Server.hpp"
#include "../includes/Client.hpp"   // ✅ OK
#include "../includes/Channel.hpp"  // ✅ OK
#include "../includes/Commands.hpp"

std::string format_privmsg(Client* sender, const std::string& target, const std::string& message)
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

bool is_channel(const std::string& target)
{
    return (!target.empty() && (target[0] == '#' || target[0] == '&'));
}

Client* Server::get_client_by_nickname(const std::string& nickname)
{
    for (int i = 1; i < g_num_fds; i++)
    {
        if (this->clients[i] && this->clients[i]->getNickname() == nickname)
        {
            return this->clients[i];
        }
    }
    return NULL;
}

void send_to_user(Server* server, Client* sender, const std::string& target_nick, const std::string& message)
{
    // 1. Trouver l'utilisateur cible
    Client* target = server->get_client_by_nickname(target_nick);
    
    // 2. Vérifier qu'il existe
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
    
    // 3. Formater le message
    std::string formatted = format_privmsg(sender, target_nick, message);
    
    // 4. Envoyer au destinataire
    send(target->get_fd(), formatted.c_str(), formatted.length(), 0);
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
    
    // Chercher le ':'
    size_t pos = argument.find(':');
    
    // Pas de ':' → ERR_NOTEXTTOSEND
    if (pos == std::string::npos)
    {
        result.error_code = 412;
        return result;
    }
    
    // Extraire destinataire
    result.target = argument.substr(0, pos);
    trim(result.target);
    
    // Destinataire vide → ERR_NORECIPIENT
    if (result.target.empty())
    {
        result.error_code = 411;
        return result;
    }
    
    // Extraire message (ne pas trim!)
    result.message = argument.substr(pos + 1);
    
    result.valid = true;
    return result;
}

void Server::handle_privmsg(int sender_index, const std::string& argument)
{
    Client* sender = this->clients[sender_index];

    // 1. Parsing
    ParsedMessage parsed = parse_arguments(argument);

    // 2. Vérifier la validité
    if (!parsed.valid)
    {
        std::string error;
        if (parsed.error_code == 411)
        {
            error = ":server 411 ";  //  Espace ajouté
            error += sender->getNickname();
            error += " :No recipient given (PRIVMSG)\r\n";
        }
        else if (parsed.error_code == 412)
        {
            error = ":server 412 ";  // Espace ajouté
            error += sender->getNickname();
            error += " :No text to send\r\n";
        }
        sendError(sender->get_fd(), error);
        return;
    }
    
    // 3. Déterminer le type de destinataire
    if (is_channel(parsed.target))
    {
        // Pour l'instant, erreur car channels pas implémentés
        std::string error = ":server 403 ";  //  Espace ajouté
        error += sender->getNickname();
        error += " ";
        error += parsed.target;
        error += " :No such channel\r\n";
        sendError(sender->get_fd(), error);
        return;
    }
    else
    {
        // C'est un utilisateur
        send_to_user(this, sender, parsed.target, parsed.message);
    }
}