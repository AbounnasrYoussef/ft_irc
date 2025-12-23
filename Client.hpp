#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index);

class Client {
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		bool _authenticated;
	public:
		Client(int fd);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();
		// getters/setters...
};
#endif