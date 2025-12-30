#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
// ✔️ khaddam logic dyalo s7i7

// ✔️ multi-client b poll

// ❌ khasso client cleanup

// ❌ khasso error handling

// ❌ khasso limits checks

// add save number of fds in client class

Server::~Server()
{
	close(this->server_Fd);
}

bool split(const std::string &s, char delimiter, std::string &left, std::string &right)
{
    std::string::size_type pos = s.find(delimiter);

    if (pos == std::string::npos)
        return false;

    left = s.substr(0, pos);
    right = s.substr(pos + 1);

    return !left.empty() && !right.empty();
}

void sendError(int fd, const std::string& msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) == -1)
		write(2, "Error sending data to client.\n", 30);
}

bool isalpha_string(std::string str)
{
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (!isalpha(str[i]))
			return false;
	}
	return true;
}

// bool user_parsing(std::string argument, Client* client)
// {
// 	size_t count = 0;
// 	for (size_t i = 0; i < argument.length(); ++i)
// 	{
// 		if (argument[i] == ' ')
// 			count++;
// 	}
// 	if (count != 3)
// 		return false;
// 	// USER <username> <hostname> <servername> :<realname>
// 	std::string username, hostname, servername, realname;
// 	size_t first_space = argument.find(' ');
// 	if (first_space == std::string::npos)
// 		return false;
// 	username = argument.substr(0, first_space);
// 	argument = argument.substr(first_space + 1);

// 	size_t second_space = argument.find(' ');
// 	if (second_space == std::string::npos)
// 		return false;
// 	hostname = argument.substr(0, second_space);
// 	argument = argument.substr(second_space + 1);

// 	size_t third_space = argument.find(' ');
// 	if (third_space == std::string::npos)
// 		return false;
// 	servername = argument.substr(0, third_space);
// 	argument = argument.substr(third_space + 1);

// 	if (argument[0] != ':')
// 		return false;
// 	realname = argument.substr(1);

// 	client->setUsername(username);
// 	client->setRealname(realname);
// 	// client->setRegistered(true);
// 	return true;
// }
#include <sstream> // for iss
std::string getClientIP(const sockaddr_storage &addr, socklen_t len) // i nedd learn from here this function 
{
	char host[NI_MAXHOST];

	if (getnameinfo((const sockaddr*)&addr, len, host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0)
	{
		return std::string(host);
	}
	return std::string();
}


bool user_parsing(const std::string& argument, Client* client) // need learn for this fuction
{
    // 1) realname must start with ':'
    size_t colonPos = argument.find(" :");
    if (colonPos == std::string::npos)
        return false;

    // 2) split into "before :" and "realname"
    std::string before = argument.substr(0, colonPos);
    std::string realname = argument.substr(colonPos + 2);

    if (realname.empty())
        return false;

    // 3) split the part before ':' into tokens
    std::istringstream iss(before);
    std::string username, hostname, servername;

    if (!(iss >> username >> hostname >> servername))
        return false;

    // 4) store values
    client->setUsername(username);
    client->setRealname(realname);

    return true;
}






























Server::Server(int port, std::string password)
{
	this->port = port;
	this->password = password;
	this->server_Fd = -1;
}

void Server::setupSocket()
{
	// this->_
	// this->_serverFd
	this->server_Fd = socket(AF_INET, SOCK_STREAM, 0); // buffering tcp create

	// 2. Bind to port 
	struct sockaddr_in address;   /// socket(ip:port) for this process 
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);
	bind(this->server_Fd, (struct sockaddr*)&address, sizeof(address));
	
	// 3. Start listening
	listen(this->server_Fd, 3); // am ready to accept connections (work) (10.50.20.7:6667)
	// this->_fds[MAX_CLIENTS];
	this->_fds[0].fd = this->server_Fd;
	this->_fds[0].events = POLLIN;
	// int g_num_fds = 1;
	
}


void Server::accept_NewClient()
{
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(this->server_Fd, (sockaddr*)&client_addr, &len);

	std::string ip = getClientIP(client_addr, len);
	// Add new client to our monitoring list

	this->_fds[g_num_fds].fd = client_fd;
	this->_fds[g_num_fds].events = POLLIN;
	this->clients[g_num_fds] = new Client(client_fd);
	this->clients[g_num_fds]->setIP(ip); // set ip of client
	
	g_num_fds++; // add to array
}


void Server::handle_ClientData(int index)
{
	//  Process client messages

	std::cout << this->clients[g_num_fds - 1]->getIP()  << std::endl;
	for (int i = 1; i < g_num_fds; i++)
	{
		if (this->_fds[i].revents & POLLIN)
		{
			// char buffer[512];
			int bytes = read(this->_fds[i].fd, this->buffer, 512);

			if (bytes == 0)
			{
				// Client disconnected
				write(this->_fds[i].fd, "Client is diconnected Goodbye!\n", 9);
				removeClient(this->_fds, clients, g_num_fds, i);
				continue;
			}

			if (bytes == -1)
			{
				// Error reading - close connection
				write(2, "Error reading data. Closing connection.\n", 41);
				close(this->_fds[i].fd);
				// TODO: Remove from array
			}
			else
			{
				this->buffer[bytes] = '\0';
				// this->clients[i]->appendBuffer(buffer);
				this->clients[i]->appendBuffer(std::string(this->buffer));
				std::string full_Buffer = this->clients[i]->getBuffer(); // 
				size_t pos;
				std::cout << this->buffer << std::endl; // for debug
				while((pos = full_Buffer.find("\r\n")) != std::string::npos)
				{
					std::string message = full_Buffer.substr(0, pos);
					full_Buffer = full_Buffer.substr(pos + 2);
					std::cout << "Complete message: [" << message << "]" << std::endl;  // for debug
					
					// handl commands and parse(PASS, NICK ,USER)

					write(this->_fds[i].fd, "Message received: ", 18);
				}
				// processCommand(this->clients[i], full_Buffer); // handle after is commenands success
				// Successfully processed command
				// if (massage_complet(this->clients[i]->getBuffer()))
				// {
				// 	// message commplet 
				// }
				// parse message
				// Process message
				write(this->_fds[i].fd, "Hello from server!\n", 19);
			}
		}
	}
}

void Server::start()
{
	// Server obj;
	setupSocket();
	while(true)
	{
		int ret = poll(this->_fds, g_num_fds, -1);  // -1 = wait forever
		if (ret == -1)
		{
			// Error - clean up and exit
			exit(0);
		}
		if(ret > 0)
		{
			// There are events to process
		
			// check for new connections
			if (this->_fds[0].revents & POLLIN)
			{
				accept_NewClient();
			}
			// check for client data
			for (int i = 1; i < g_num_fds; i++)
			{
				if (this->_fds[i].revents & POLLIN)
				{
					handle_ClientData(i);
				}
			}
		}
	}
	
}