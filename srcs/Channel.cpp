#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include <unistd.h>

// ─────────────────────────────────────────────
//  Constructeurs / Destructeur
// ─────────────────────────────────────────────

Channel::Channel()
    : _name(""), _topic(""), _key(""), _hasKey(false),
      _invite_only(false), _topic_protected(false),
      _moderated(false), _no_external(false), _user_limit(0)
{}

Channel::Channel(const std::string &name)
    : _name(name), _topic(""), _key(""), _hasKey(false),
      _invite_only(false), _topic_protected(false),
      _moderated(false), _no_external(false), _user_limit(0)
{}

Channel::Channel(const Channel &other)
{
    _name      = other._name;
    _members   = other._members;
    _operators = other._operators;
    _topic     = other._topic;
    _key       = other._key;
    _hasKey    = other._hasKey;
    _invite_only     = other._invite_only;
    _topic_protected = other._topic_protected;
    _moderated       = other._moderated;
    _no_external     = other._no_external;
    _user_limit      = other._user_limit;
}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        _name      = other._name;
        _members   = other._members;
        _operators = other._operators;
        _topic     = other._topic;
        _key       = other._key;
        _hasKey    = other._hasKey;
        _invite_only     = other._invite_only;
        _topic_protected = other._topic_protected;
        _moderated       = other._moderated;
        _no_external     = other._no_external;
        _user_limit      = other._user_limit;
    }
    return *this;
}

Channel::~Channel()
{}

// ─────────────────────────────────────────────
//  Gestion membres — interface unifiée
//  (remplace l'ancienne dualité _users / _members)
// ─────────────────────────────────────────────

void Channel::addUser(Client *client)
{
    if (!client)
        return;
    if (has_member(client))
        return;

    _members.push_back(client);
    client->addChannel(this);

    // Le premier membre devient opérateur automatiquement
    if (_members.size() == 1)
        add_operator(client);
}

bool Channel::hasUser(Client *client) const
{
    for (size_t i = 0; i < _members.size(); i++)
        if (_members[i] == client)
            return true;
    return false;
}

void Channel::removeUser(Client *client)
{
    remove_member(client); // retire de _members et _operators
}

std::string Channel::getuserList()
{
    std::string list;
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (!list.empty())
            list += " ";
        // Préfixe '@' pour les opérateurs
        if (is_operator(_members[i]))
            list += "@";
        list += _members[i]->getNickname();
    }
    return list;
}

// ─────────────────────────────────────────────
//  Broadcast
// ─────────────────────────────────────────────

void Channel::broadcast(const std::string &message, Client *exclude)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == exclude)
            continue;
        send(_members[i]->get_fd(), message.c_str(), message.size(), 0);
    }
}

// ─────────────────────────────────────────────
//  Clé de canal (+k)
// ─────────────────────────────────────────────

void Channel::setAkey(const std::string &key)
{
    _key    = key;
    _hasKey = !key.empty();
}

const std::string &Channel::getkey() const
{
    return _key;
}

bool Channel::hasAkeys() const
{
    return _hasKey;
}

bool Channel::checkAkey(const std::string &key) const
{
    return (key == _key);
}

// ─────────────────────────────────────────────
//  Membres (accès direct pour KICK / MODE)
// ─────────────────────────────────────────────

void Channel::add_member(Client *client)
{
    if (!has_member(client))
        _members.push_back(client);
}

void Channel::remove_member(Client *client)
{
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] == client)
        {
            _members.erase(_members.begin() + i);
            break;
        }
    }
    // Retirer aussi des opérateurs
    remove_operator(client);
}

bool Channel::has_member(Client *client)
{
    for (size_t i = 0; i < _members.size(); i++)
        if (_members[i] == client)
            return true;
    return false;
}

std::vector<Client *> Channel::get_members()
{
    return _members;
}

// ─────────────────────────────────────────────
//  Opérateurs
// ─────────────────────────────────────────────

void Channel::add_operator(Client *client)
{
    if (!is_operator(client))
        _operators.push_back(client);
}

void Channel::remove_operator(Client *client)
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            return;
        }
    }
}

bool Channel::is_operator(Client *client)
{
    for (size_t i = 0; i < _operators.size(); i++)
        if (_operators[i] == client)
            return true;
    return false;
}

// ─────────────────────────────────────────────
//  Topic
// ─────────────────────────────────────────────

std::string Channel::get_topic()
{
    return _topic;
}

void Channel::set_topic(std::string topic)
{
    _topic = topic;
}

// ─────────────────────────────────────────────
//  Invite
// ─────────────────────────────────────────────

void Channel::addUserInvite(Client *client)
{
    if (!client)
        return;
    _invited.insert(client);
}

bool Channel::isInvited(Client *client) const
{
    return (_invited.find(client) != _invited.end());
}

// ─────────────────────────────────────────────
//  Infos générales
// ─────────────────────────────────────────────

std::string Channel::get_name()
{
    return _name;
}

bool Channel::is_empty()
{
    return _members.empty();
}

// ─────────────────────────────────────────────
//  Modes
// ─────────────────────────────────────────────

bool Channel::is_invite_only()     const { return _invite_only; }
bool Channel::is_topic_protected() const { return _topic_protected; }
bool Channel::is_moderated()       const { return _moderated; }
bool Channel::is_no_external()     const { return _no_external; }
int  Channel::get_user_limit()     const { return _user_limit; }

void Channel::set_invite_only(bool v)     { _invite_only = v; }
void Channel::set_topic_protected(bool v) { _topic_protected = v; }
void Channel::set_moderated(bool v)       { _moderated = v; }
void Channel::set_no_external(bool v)     { _no_external = v; }
void Channel::set_user_limit(int limit)   { _user_limit = limit; }

// ─────────────────────────────────────────────
//  Méthodes Server liées aux channels
// ─────────────────────────────────────────────

Channel *Server::get_channel(const std::string &name)
{
    std::map<std::string, Channel *>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return it->second;
    return NULL;
}

Channel *Server::create_channel(const std::string &name)
{
    Channel *existing = get_channel(name);
    if (existing)
        return existing;
    Channel *newChan = new Channel(name);
    _channels[name]  = newChan;
    return newChan;
}

void Server::delete_channel(Channel *channel)
{
    if (!channel)
        return;
    std::string name = channel->get_name();
    _channels.erase(name);
    delete channel;
}
