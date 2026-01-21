#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <iostream>
#include "Server.hpp"
#include <set> // OTHMANE ADDTHIS FOR join
#include <sstream> // for iss
#include <string>
#include "Channel.hpp"
class Channel;
class Client;
void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index);
std::string getClientIP(const sockaddr_storage &addr, socklen_t len);
// bool massage_complet(std::string buffer);
bool user_parsing(const std::string& argument, Client* client);
bool pars_nick(std::string _nickname);
bool split(std::string &s, char delimiter, std::string &left, std::string &right);

class Client {
	private:
		int _fd;                    // Client file descriptor
		std::string _nickname;      // Client nickname
		std::string _username;      // Client username
		std::string _realname;      // Real name
		std::string _buffer;        // Message buffer (for partial messages)
		std::string _ip;   
		std::set<Channel*> _channels;           // IP of client 
		bool _passOk;               // Password authenticated?
		bool _welcomeSent; 		// Welcome message sent?
		// bool _registered;           // Fully registered? (PASS + NICK + USER) // no need
		
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
		void setIP(std::string ip);
		void setPassOk(bool ok);
		bool isWelcomeSent() const;
		void setWelcomeSent(bool sent);
		// void setRegistered(bool reg); no need
		// Buffer management
		void appendBuffer(std::string const data);
		void clearBuffer();
		void setBuffer(std::string const data);
		// bool isNicknameTaken(std::string nickname);


		// for join 
		// Client(const std::string& nick,const std::string& user);
		void addChannel(Channel *Channel);
		// void removeChannel(Channel *Channel);
		// bool isInChannel(const std::string& channelName)const;

		// const std::string& getNickname() const;
		// const std::string& getUsername() const; 
		// const std::set<Channel*> getChannels() const;
		
};
#endif