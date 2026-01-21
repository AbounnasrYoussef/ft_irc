#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#define MAX_CLIENTS 10

#include <vector>
#include <map>
#include "Channel.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

// add for othmane

#include "Channel.hpp"

#include <stdlib.h>
#include <netdb.h>		// getnameinfo, NI_MAXHOST
#include <map>
#include <sys/socket.h> // sockaddr, sockaddr_storage
#include <netinet/in.h> // sockaddr_in, sockaddr_in6
#include "Channel.hpp"
#include <arpa/inet.h>	// AF_INET, AF_INET6

class Client;
class Channel;
extern int g_num_fds;

void sendError(int fd, const std::string &msg);
bool isalpha_string(std::string str);

class Server
{
private:
	int server_Fd;					 // Server socket fd
	int port;						 // Port number
	std::map<std::string, Channel *> _channels;
	std::string password;			 // Server password
	char buffer[512];				 // Buffer for incoming data
	struct pollfd _fds[MAX_CLIENTS]; // Poll array
	// int _numFds;                      // Number of active fds


public:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);
	Server(int port, std::string password);
	~Server();

	Client *clients[MAX_CLIENTS]; // Array of client pointers

	// Core server functions
	void start();					   // Main server loop
	void setupSocket();				   // socket() + bind() + listen()
	void accept_NewClient();		   // accept() new connection
	void handle_ClientData(int index); // Process client messages
	// void removeClient(int index);     // Disconnect and cleanup

	// Message handling
	// void processCommand(Client* client, std::string message);
	// void broadcastToChannel(std::string channelName, std::string message, Client* sender);
	void processCommand(int index, std::string &message);

	// Utilities
	Client *getClientByNick(std::string nickname);
	bool isNicknameTaken(std::string nickname, int excludeIndex);
	bool check_passok(std::string command, std::string argument, int index);
	bool check_authentication(std::string command, std::string argument, int index);

	// for add channel
	Channel *findOrCreateChannel(const std::string &name);

	// youssef part
	void handle_privmsg(int sender_index, const std::string &argument);
	Client *get_client_by_nickname(const std::string &nickname);
	Channel *get_channel(const std::string &name);
	Channel *create_channel(const std::string &name);
	void delete_channel(Channel *channel);
	void handle_kick(int kicker_index, const std::string &argument);
	void handle_mode(int setter_index, const std::string &argument);
};

#endif