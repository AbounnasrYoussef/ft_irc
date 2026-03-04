#include "../includes/Client.hpp"

class channel;

Client::Client(int fd) : _fd(fd)
{
	_nickname = "";
	_username = "";
	_realname = "";
	_buffer = "";
	_passOk = false;
	_welcomeSent = false;
}

Client::~Client()
{
}

// Getters
int Client::get_fd() const
{
	return _fd;
}

std::string Client::getNickname() const
{
	return _nickname;
}

std::string Client::getUsername() const
{
	return _username;
}

bool Client::isRegistered() const
{
	return (_passOk && !_nickname.empty() && !_username.empty());
}

bool Client::isWelcomeSent() const
{
	return _welcomeSent;
}
void Client::setWelcomeSent(bool sent)
{
	_welcomeSent = sent;
}

std::string Client::getIP() const
{
	return _ip;
}

// void Client::setRegistered(bool reg) // no need
// {
// 	_registered = reg;
// }

bool Client::isPassOk() const
{
	return _passOk;
}

std::string Client::getBuffer() const
{
	return _buffer;
}

// Setters

void Client::setNickname(std::string nick)
{
	_nickname = nick;
}

void Client::setUsername(std::string user)
{
	_username = user;
}

void Client::appendBuffer(std::string const data)
{
	_buffer += data;
}

void Client::setRealname(std::string real)
{
	_realname = real;
}

void Client::setPassOk(bool ok)
{
	_passOk = ok;
}

void Client::setIP(std::string ip)
{
	this->_ip = ip;
}

void Client::clearBuffer()
{
	_buffer.clear();
}

void Client::setBuffer(std::string const data)
{
	_buffer = data;
}

void Client::addChannel(Channel *channel)
{
	if (!channel)
	{
		return;
	}
	if (_channels.find(channel) != _channels.end())
	{
		return;
	}
	_channels.insert(channel);
	// client->addChannel(this);
}

// void Client::removeChannel(Channel *chanael)
// {
// 	if(!chanael)
// 		{
// 			return;
// 		}
// 		auto it = Chanael.find(chanael);
// 		if(it != Chanael.end())
// 		{
// 			Chanael.erase(it);
// 		}

// }
// 	bool Client::isInChannel(const std::string& channelName)const
// 	{
// 		    for (std::set<Channel*>::const_iterator it = Chanael.begin();
//          it != Chanael.end(); ++it)
//     {
//         if ((*it)->getName() == channelName)
//             return true;
//     }
//     return false;
// 	}
// const std::set<Channel*> Client::getChannels() const
// {
// 	return Chanael;
// }

std::string Client::get_prefix() const
{
	return _nickname + "!" + _username + "@" + _ip;
}