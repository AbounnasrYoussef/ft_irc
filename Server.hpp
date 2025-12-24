#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>

class Server {
	private:
		int _port;
		std::string _password;
	public:
		Server();
		Server(int port, const std::string& password);
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();
		// getters/setters...
};

#endif