#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
# define MAX_CLIENTS 10

#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <netdb.h>       // getnameinfo, NI_MAXHOST
#include <sys/socket.h>  // sockaddr, sockaddr_storage
#include <netinet/in.h>  // sockaddr_in, sockaddr_in6
#include <arpa/inet.h>   // AF_INET, AF_INET6

class Client;
extern int g_num_fds;

void sendError(int fd, const std::string& msg);
bool isalpha_string(std::string str);
void processCommand(Client* client, std::string message);

class Server {
	private:
		int _serverFd;                    // Server socket fd
		int _port;                        // Port number
		std::string _password;            // Server password
		struct pollfd _fds[MAX_CLIENTS];  // Poll array
		Client* _clients[MAX_CLIENTS];    // Array of client pointers
		int _numFds;                      // Number of active fds
	
	public:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);
		Server(int port, std::string password);
		~Server();
	
	// Core server functions
	void start();                     // Main server loop
	void setupSocket();               // socket() + bind() + listen()
	void acceptNewClient();           // accept() new connection
	void handleClientData(int index); // Process client messages
	// void removeClient(int index);     // Disconnect and cleanup
	
	// Message handling
	// void processCommand(Client* client, std::string message);
	void broadcastToChannel(std::string channelName, std::string message, Client* sender);
	
	// Utilities
	Client* getClientByNick(std::string nickname);
	bool isNicknameTaken(std::string nickname);
};


#endif