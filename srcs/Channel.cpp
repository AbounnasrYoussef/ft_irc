#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

Channel::Channel() : _name(""), _topic(""), _key("")
{
}

Channel::Channel(const std::string& name) : _name(name), _topic(""), _key("")
{
}

Channel::Channel(const Channel& other)
{
    _name = other._name;
    _members = other._members;
    _operators = other._operators;
    _topic = other._topic;
    _key = other._key;
}

Channel& Channel::operator=(const Channel& other)
{
    if (this != &other)
    {
        _name = other._name;
        _members = other._members;
        _operators = other._operators;
        _topic = other._topic;
        _key = other._key;
    }
    return *this;
}

Channel::~Channel()
{
}

// Gestion des membres
void Channel::add_member(Client* client)
{
    if (!has_member(client))
    {
        _members.push_back(client);
    }
}

void Channel::remove_member(Client* client)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == client)
        {
            _members.erase(_members.begin() + i);
            break;
        }
    }
    
    // Retirer aussi des opérateurs si c'était un op
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            break;
        }
    }
}

bool Channel::has_member(Client* client)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == client)
            return true;
    }
    return false;
}

std::vector<Client*> Channel::get_members()
{
    return _members;
}

// Gestion des opérateurs
void Channel::add_operator(Client* client)
{
    if (!is_operator(client))
    {
        _operators.push_back(client);
    }
}

void Channel::remove_operator(Client* client)
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            break;
        }
    }
}

bool Channel::is_operator(Client* client)
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == client)
            return true;
    }
    return false;
}

// Getters/Setters
std::string Channel::get_name()
{
    return _name;
}

std::string Channel::get_topic()
{
    return _topic;
}

void Channel::set_topic(std::string topic)
{
    _topic = topic;
}

bool Channel::is_empty()
{
    return _members.empty();
}

// Fonctions du serveur (à déplacer dans un fichier séparé)
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
    Channel* existing = get_channel(name);
    if (existing)
        return existing;
    
    Channel* new_channel = new Channel(name);
    _channels[name] = new_channel;
    return new_channel;
}

void Server::delete_channel(Channel* channel)
{
    if (!channel)
        return;
    
    std::string name = channel->get_name();
    _channels.erase(name);
    delete channel;
}