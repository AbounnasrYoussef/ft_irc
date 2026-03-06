#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "fcntl.h"

// #include "Client.hpp"
class Channel;
class Client;

Server::~Server()
{
	// Clean up all clients
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i])
		{
			close(clients[i]->get_fd());
			delete clients[i];
			clients[i] = NULL;
		}
	}
	clients.clear();
	
	// Clean up all channels
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); 
	     it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	
	// Close server socket
	if (this->server_Fd != -1)
		close(this->server_Fd);
}

Server::Server(int port, std::string password)
{
	this->port = port;
	this->password = password; // add \r\n to password for comparison
	this->server_Fd = -1;
}

bool Server::setupSocket()
{
	int pp = 1;
	this->server_Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_Fd == -1)
	{
		std::cerr << "Error: Failed to create socket" << std::endl;
		return false;
	}
	if (fcntl(this->server_Fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: Failed to set server socket to non-blocking" << std::endl;
		close(this->server_Fd);
		return false;
	}
	if (setsockopt(this->server_Fd, SOL_SOCKET, SO_REUSEADDR, &pp, sizeof(int)) == -1) // to reuse address immediately after close
	{
		std::cerr << "Error: Failed setsockopt" << std::endl;
		close(this->server_Fd);
		return false;
	}
	// 2. Bind to port
	struct sockaddr_in address; /// socket(ip:port) for this process
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);

	if (bind(this->server_Fd, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		std::cerr << "Error: Failed to bind to port " << this->port << std::endl;
		close(this->server_Fd);
		return false;
	}

	// 3. Start listening
	if (listen(this->server_Fd, 3) == -1) // am ready to accept connections (work) (10.50.20.7:6667)
	{
		std::cerr << "Error: Failed to listen on socket " << this->port << std::endl;
		close(this->server_Fd);
		return false;
	}
	struct pollfd serverEntry;
	serverEntry.fd = this->server_Fd;
	serverEntry.events = POLLIN;
	serverEntry.revents = 0;
	this->_fds.push_back(serverEntry);
	this->clients.push_back(NULL); // placeholder so indices align (clients[0] = server slot)
	return true;
}

void Server::accept_NewClient()
{
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(this->server_Fd, (sockaddr *)&client_addr, &len);
	if (client_fd == -1)
		return;

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: Failed to set client socket to non-blocking" << std::endl;
		close(client_fd);
		return;
	}

	std::string ip = getClientIP(client_addr, len);

	struct pollfd clientEntry;
	clientEntry.fd = client_fd;
	clientEntry.events = POLLIN;
	clientEntry.revents = 0;

	this->_fds.push_back(clientEntry);
	Client* newClient = new Client(client_fd);

	newClient->setIP(ip);
	this->clients.push_back(newClient);
}

void Server::handle_ClientData(int index)
{

	if (this->_fds[index].revents & POLLIN)
	{
		int bytes = recv(this->_fds[index].fd, this->buffer, 511, 0);
		if (bytes == 0)
		{
			// Client disconnected
			removeClient(this->_fds, clients, index);
			return;
		}

		if (bytes == -1)
		{
			// Error reading - close connection
			std::cerr << "Error reading data from client: " << strerror(errno) << std::endl;
			removeClient(this->_fds, clients, index);
			return;
		}

		// Null terminate the buffer
		this->buffer[bytes] = '\0';

		// Append new data to client's buffer
		this->clients[index]->appendBuffer(std::string(this->buffer, bytes));

		// Get the full buffer and process complete messages
		std::string full_Buffer = this->clients[index]->getBuffer();

		if (full_Buffer.length() > 512)
		{
			std::string error_msg = "ERROR :Message too long (max 512 bytes)\r\n";
			send(this->_fds[index].fd, error_msg.c_str(), error_msg.length(), 0);
			removeClient(this->_fds, clients, index);
			return;
		}
		
		size_t pos;
		// Process all complete messages (ending with \r\n)
		while ((pos = full_Buffer.find("\r\n")) != std::string::npos)
		{
			std::string message = full_Buffer.substr(0, pos);
			// Remove processed message from buffer
			full_Buffer = full_Buffer.substr(pos + 2);
			while (!message.empty())
				processCommand(index, message);
		}

		// Update client's buffer with remaining unprocessed data
		this->clients[index]->setBuffer(full_Buffer);
	}
}

void Server::start()
{
	if (!setupSocket())
		return;
	while (true)
	{	
		// NEW CODE - Handle EINTR (interrupted system call) properly
		int ret = poll(&this->_fds[0], this->_fds.size(), -1);
		if (ret == -1)
		{
			if (errno == EINTR)
			{
				continue; // Interrupted by signal, retry poll()
			}
			std::cerr << "Error: poll() failed: " << strerror(errno) << std::endl;
			break; // Exit loop, destructor will clean up properly
		}
		if (ret > 0)
		{
			// There are events to process

			// check for new connections
			if (this->_fds[0].revents & POLLIN)
			{
				accept_NewClient();
			}
			// check for client data
			for (int i = 1; i < (int)this->_fds.size(); i++)
			{
				if (this->_fds[i].revents & POLLIN)
				{
					handle_ClientData(i);
				}
				else if (this->_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
				{
					removeClient(this->_fds, clients, i);
					i--; // Adjust index after removal
				}
			}
		}
	}
}

// function to add and creat channel of the JOIN

Channel *Server::findOrCreateChannel(const std::string &name)
{
	if (_channels.find(name) != _channels.end())
	{
		return _channels[name];
	}
	else
	{
		Channel *newChan = new Channel(name);
		_channels[name] = newChan;
		return newChan;
	}
}
bool Server::findChannel(const std::string &name)
{
	if (_channels.find(name) != _channels.end())
	{
		return true;
	}
	else
		return false;
}
Client *Server::findClient(const std::string &nickname)
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client *client = it->second;
		if (!client)
		{
			continue;
		}
		if (client->getNickname() == nickname)
		{
			return client;
		}
	}
	return nullptr;
}