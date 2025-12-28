#include "includes/Client.hpp"

Client::Client(int fd) : _fd(fd)
{
	_nickname = "";
	_username = "";
	_realname = "";
	_buffer = "";
	_passOk = false;
	_registered = false;
}
Client::~Client() {}

// Getters
// int Client::getFd() const {
// 	return _fd;
// }
// std::string Client::getNickname() const {
// 	return _nickname;
// }

// std::string Client::getUsername() const {
// 	return _username;
// }

bool Client::isRegistered() const {
	return _registered;
}

void Client::setRegistered(bool reg) {
	_registered = reg;
}

// bool Client::isPassOk() const {
// 	return _passOk;
// }

std::string Client::getBuffer() const {
	return _buffer;
}

// Setters

// void Client::setNickname(std::string nick) {
// 	_nickname = nick;
// }

// void Client::setUsername(std::string user) {
// 	_username = user;
// }

void Client::appendBuffer(std::string const data) {
	_buffer += data;
}

bool Client::isNicknameTaken(std::string nickname)
{
	for (int i = 1; i < this->_numFds; i++)
	{
		if (this->getNickname() == nickname)
			return true;
	}
	return false;
}

bool pars_nick(std::string _nickname)
{
	// Nickname must start with a letter or special character
	if (isalpha_string(_nickname))
		return true;

	// // Check each character in the nickname
	// for (size_t i = 1; i < _nickname.length(); ++i)
	// {
	// 	char c = _nickname[i];
	// 	if (!isalnum(c) && !strchr("[]\\`_^{|}-", c))
	// 		return true;
	// }

	return false;
}
void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index)
{
	// Close the connection
	close(fds[index].fd);
	
	// Delete the Client object
	delete clients[index];
	clients[index] = NULL;
	
	// Shift both arrays
	for (int i = index; i < num_fds - 1; i++) {
		fds[i] = fds[i + 1];
		clients[i] = clients[i + 1];
	}
	
	// Decrease count
	num_fds--;
}
