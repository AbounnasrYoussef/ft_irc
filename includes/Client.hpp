#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>
#include "Server.hpp"


// void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index);
std::string getClientIP(const sockaddr_storage &addr, socklen_t len);
bool massage_complet(std::string buffer);
bool user_parsing(std::string argument, Client* client);


class Client {
	private:
		int _fd;                    // Client file descriptor
		std::string _nickname;      // Client nickname
		std::string _username;      // Client username
		std::string _realname;      // Real name
		std::string _buffer;        // Message buffer (for partial messages)
		std::string _ip;                // IP of client 
		bool _passOk;               // Password authenticated?
		bool _registered;           // Fully registered? (PASS + NICK + USER)
		
	public:
		Client(int fd);
		~Client();
		std::string _password;            // Password
		int _numFds; // number of active fds
		
		// Getters
		int get_fd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		bool isRegistered() const;
		bool isPassOk() const;
		std::string getBuffer() const;
		std::string getIP() const;
		
		// Setters
		void setNickname(std::string nick);
		void setUsername(std::string user);
		void setRealname(std::string real);
		void setPassOk(bool ok);
		void setRegistered(bool reg);
		// Buffer management
		void appendBuffer(std::string const data);
		bool isNicknameTaken(std::string nickname);
		// void clearBuffer();
};

#endif