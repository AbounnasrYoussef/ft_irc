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

