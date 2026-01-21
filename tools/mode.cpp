#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"

// Fonction pour afficher les modes actuels
void send_channel_modes(Client* client, Channel* channel)
{
    std::string modes = "+";
    
    if (channel->is_invite_only()) modes += "i";
    if (channel->is_topic_protected()) modes += "t";
    if (channel->is_moderated()) modes += "m";
    if (channel->is_no_external()) modes += "n";
    
    if (modes == "+")
        modes = "+";  // Pas de modes
    
    std::string response = ":server 324 ";
    response += client->getNickname();
    response += " ";
    response += channel->get_name();
    response += " ";
    response += modes;
    response += "\r\n";
    
    send(client->get_fd(), response.c_str(), response.length(), 0);
}

ParsedMode parse_mode_arguments(const std::string& argument)
{
    ParsedMode result;
    result.valid = false;
    
    // 1. Split par espace
    size_t space_pos = argument.find(' ');
    
    if (space_pos == std::string::npos)
    {
        // Juste le channel, pas de modes (consultation)
        result.channel = argument;
        trim(result.channel);
        result.valid = true;
        return result;
    }
    
    result.channel = argument.substr(0, space_pos);
    trim(result.channel);
    
    std::string modes_and_params = argument.substr(space_pos + 1);
    trim(modes_and_params);
    
    // 2. Parser la chaîne de modes (ex: "+it", "-o", "+k")
    if (modes_and_params.empty())
    {
        result.valid = true;
        return result;
    }
    
    // 3. Séparer les modes des paramètres
    size_t next_space = modes_and_params.find(' ');
    std::string mode_string;
    std::string params_string;
    
    if (next_space == std::string::npos)
    {
        mode_string = modes_and_params;
        params_string = "";
    }
    else
    {
        mode_string = modes_and_params.substr(0, next_space);
        params_string = modes_and_params.substr(next_space + 1);
    }
    
    // 4. Parser la chaîne de modes caractère par caractère
    bool adding = true;  // Par défaut +
    
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
            // C'est un mode (i, t, o, k, l, etc.)
            ModeChange change;
            change.mode = c;
            change.adding = adding;
            change.param = "";
            
            result.changes.push_back(change);
        }
    }
    
    // 5. Associer les paramètres aux modes qui en ont besoin
    // Pour l'instant, on laisse vide, on le fera après
    
    result.valid = true;
    return result;
}

void Server::handle_mode(int setter_index, const std::string& argument)
{
    Client* setter = this->clients[setter_index];
    
    // 1. Parser
    ParsedMode parsed = parse_mode_arguments(argument);
    
    if (!parsed.valid)
    {
        std::string error = ":server 461 ";
        error += setter->getNickname();
        error += " MODE :Not enough parameters\r\n";
        sendError(setter->get_fd(), error);
        return;
    }
    
    // 2. Trouver le channel
    Channel* channel = this->get_channel(parsed.channel);
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
    
    // 3. Si pas de modes, afficher les modes actuels (consultation)
    if (parsed.changes.empty())
    {
        send_channel_modes(setter, channel);
        return;
    }
    
    // 4. Vérifier que setter est dans le channel
    if (!channel->has_member(setter))
    {
        std::string error = ":server 442 ";
        error += setter->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not on that channel\r\n";
        sendError(setter->get_fd(), error);
        return;
    }
    
    // 5. Vérifier que setter est opérateur
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
    
    // 6. Appliquer chaque mode
    std::string applied_modes = "";
    
    for (size_t i = 0; i < parsed.changes.size(); i++)
    {
        ModeChange& change = parsed.changes[i];
        
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
        else
        {
            // Mode inconnu, ignorer ou erreur
            std::string error = ":server 472 ";
            error += setter->getNickname();
            error += " ";
            error += change.mode;
            error += " :is unknown mode char to me\r\n";
            sendError(setter->get_fd(), error);
        }
    }
    
    // 7. Notifier tous les membres
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
        mode_msg += "\r\n";
        
        std::vector<Client*> members = channel->get_members();
        for (size_t i = 0; i < members.size(); i++)
        {
            send(members[i]->get_fd(), mode_msg.c_str(), mode_msg.length(), 0);
        }
    }
}

