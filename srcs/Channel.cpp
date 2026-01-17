#include "includes/Channel.hpp"
#include "includes/Server.hpp"


Channel::Channel(const std::string& name)
{
    _name = name;
}

Channel* Server::get_channel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end())
    {
        return it->second;
    }
    return NULL;
}

Channel* Server::create_channel(const std::string& name)
{
    // Vérifier si le channel existe déjà
    Channel* existing = get_channel(name);
    if (existing)
        return existing;
    
    // Créer un nouveau channel
    Channel* new_channel = new Channel(name);
    _channels[name] = new_channel;
    return new_channel;
}

void Server::delete_channel(Channel* channel)
{
    if (!channel)
        return;
    
    std::string name = channel->get_name();
    
    // Retirer du map
    _channels.erase(name);
    
    // Libérer la mémoire
    delete channel;
}
std::string Channel::get_name()
{
    return _name;
}
Channel::~Channel()
{
}