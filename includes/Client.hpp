#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>

// void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index);

bool massage_complet(std::string buffer);
class Client {
	private:
		int _fd;                    // Client file descriptor
		std::string _nickname;      // Client nickname
		std::string _username;      // Client username
		std::string _realname;      // Real name
		std::string _buffer;        // Message buffer (for partial messages)
		bool _passOk;               // Password authenticated?
		bool _registered;           // Fully registered? (PASS + NICK + USER)
		
	public:
		Client(int fd);
		~Client();
		
		// Getters
		int getFd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		bool isRegistered() const;
		bool isPassOk() const;
		std::string getBuffer() const;
		
		// Setters
		void setNickname(std::string nick);
		void setUsername(std::string user);
		void setRealname(std::string real);
		void setPassOk(bool ok);
		void setRegistered(bool reg);
		
		// Buffer management
		void appendBuffer(std::string const data);
		void clearBuffer();
};

#endif