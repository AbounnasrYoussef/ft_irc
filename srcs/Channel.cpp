#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

#include <unistd.h>

class Channel;
Channel::Channel(const std::string &name) : _name(name), _topic(""), _key(""), hasAkey(false)
{
        _invite_only = false;
    _topic_protected = false;
    _moderated = false;
    _no_external = false;
    _user_limit = 0;

}

void Channel::addUser(Client *client)
{
	if (!client)
	{
		return;
	}
	if (_users.find(client) != _users.end())
	{
		return;
	}
	_users.insert(client);
	client->addChannel(this);
}

// void Channel::removeUser(Client *client)
// {
// 	if(!client)
// 		{
// 			return;
// 		}
// 		auto it = _users.find(client);
// 		if(it != _users.end())
// 		{
// 			_users.erase(it);
// 			client->removeChannel(this);
// 		}
// }

bool Channel::hasUser(Client *Client) const
{

	return (_users.find(Client) != _users.end());
}
void Channel::broadcast(const std::string &message, Client *exclude)
{
	for (Client *client : _users)
	{
		if (client == exclude)
		{
			continue;
		};
		send(client->get_fd(), message.c_str(), message.size(), 0);
	}
}
void Channel::setAkey(const std::string &key)
{
	_key = key;
	hasAkey = true;
}
const std::string &Channel::getkey() const
{
	return _key;
}
bool Channel::hasAkeys() const
{
	return hasAkey;
}
bool Channel::checkAkey(const std::string &key) const
{
	return (key == _key);
}
// const std::string& Channel::getName() const {
// 	return _name;
// }
// const std::set<Client*>& Channel::getUsers() const
// {
// return _users;
// }
// #include "../includes/Channel.hpp"

// Channel::Channel(const std::string& name)
// 	: _name(name)
// {
// }

// Channel::~Channel()
// {
// }
std::string Channel::getuserList()
{
	std::string list;
	for (std::set<Client *>::iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (!list.empty())
			list += " ";
		list += (*it)->getNickname();
	}
	return list;
}
// #include "../includes/Server.hpp"

// Channel::Channel(const std::string& name) : _name(name), _key("")
// {
//     _invite_only = false;
//     _topic_protected = false;
//     _moderated = false;
//     _no_external = false;
//     _user_limit = 0;
// }


// Channel::Channel(const std::string& name) : _name(name), _topic(""), _key("")
// {
// }

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
// Getters
bool Channel::is_invite_only() const { return _invite_only; }
bool Channel::is_topic_protected() const { return _topic_protected; }
bool Channel::is_moderated() const { return _moderated; }
bool Channel::is_no_external() const { return _no_external; }
int Channel::get_user_limit() const { return _user_limit; }

// Setters
void Channel::set_invite_only(bool value) { _invite_only = value; }
void Channel::set_topic_protected(bool value) { _topic_protected = value; }
void Channel::set_moderated(bool value) { _moderated = value; }
void Channel::set_no_external(bool value) { _no_external = value; }
void Channel::set_user_limit(int limit) { _user_limit = limit; }

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