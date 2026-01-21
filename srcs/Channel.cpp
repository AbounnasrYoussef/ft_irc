#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include <unistd.h>

class Channel;
Channel::Channel(const std::string &name) : _name(name), _key(""), hasAkey(false)
{
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