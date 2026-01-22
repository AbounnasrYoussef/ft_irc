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
#include "fcntl.h"


Server::~Server()
{
	close(this->server_Fd);
}

Server::Server(int port, std::string password)
{
	this->port = port;
	this->password = password; // add \r\n to password for comparison
	this->server_Fd = -1;
}

void Server::setupSocket()
{
	// this->_
	// this->_serverFd
	int pp = 1;
	this->server_Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_Fd == -1)
	{
		std::cerr << "Error: Failed to create socket" << std::endl;
		// Cleanup and exit
		exit(1);
	}
    if (fcntl(this->server_Fd, F_SETFL, O_NONBLOCK) == -1) 
	{
		std::cerr << "Error: Failed to set server socket to non-blocking" << std::endl;
		close(this->server_Fd);
		// Cleanup and exit
		exit(1);
	}
	if (setsockopt(this->server_Fd, SOL_SOCKET, SO_REUSEADDR, &pp, sizeof(int)) == -1) // to reuse address immediately after close
	{
		std::cerr << "Error: Failed setsockopt" << std::endl;
		close(this->server_Fd);
		// Cleanup and exit
		exit(1);
	} 
	// 2. Bind to port 
	struct sockaddr_in address;   /// socket(ip:port) for this process 
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);

	if (bind(this->server_Fd, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		std::cerr << "Error: Failed to bind to port " << this->port << std::endl;
		close(this->server_Fd);
		// Cleanup and exit
		exit(1);
	}
	
	// 3. Start listening
	if (listen(this->server_Fd, 3) == -1) // am ready to accept connections (work) (10.50.20.7:6667)
	{
		std::cerr << "Error: Failed to listen on socket " << this->port << std::endl;
		close(this->server_Fd);
		// Cleanup and exit
		exit(1);
	}
	// 4. Initialize pollfd structure
	this->_fds[0].fd = this->server_Fd;
	this->_fds[0].events = POLLIN;
	this->_fds[0].revents = 0;
	
	for (int i = 1; i < MAX_CLIENTS + 1; i++)
	{
		this->_fds[i].fd = -1;
		this->_fds[i].events = 0;
		this->_fds[i].revents = 0;
	}
}


void Server::accept_NewClient()
{
	if (g_num_fds >= MAX_CLIENTS)
	{
		// Accept and immediately close to prevent client from waiting
		sockaddr_storage client_addr;
		socklen_t len = sizeof(client_addr);
		int client_fd = accept(this->server_Fd, (sockaddr*)&client_addr, &len);
		if (client_fd != -1)
		{
			sendError(client_fd, "ERROR : Server is full\r\n");
			close(client_fd);
		}
		return;
	}
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(this->server_Fd, (sockaddr*)&client_addr, &len);
	if (client_fd == -1)
	{
		// Error accepting new client
		return;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) 
	{
		std::cerr << "Error: Failed to set client socket to non-blocking" << std::endl;
		close(client_fd);
		return;
	}
	std::string ip = getClientIP(client_addr, len);
	// Add new client to our monitoring list

	this->_fds[g_num_fds].fd = client_fd;
	this->_fds[g_num_fds].events = POLLIN;
	this->_fds[g_num_fds].revents = 0;
	this->clients[g_num_fds] = new Client(client_fd);
	this->clients[g_num_fds]->setIP(ip); // set ip of client
	
	g_num_fds++; // add to array
}


void Server::handle_ClientData(int index)
{
	//  Process client messages for the specific client at index

	if (this->_fds[index].revents & POLLIN) // need check pollout too
	{
		int bytes = read(this->_fds[index].fd, this->buffer, 511); // Read max 511 to leave room for null terminator

		if (bytes == 0)
		{
			// Client disconnected
			removeClient(this->_fds, clients, g_num_fds, index);
			return;
		}

		if (bytes == -1)
		{
			// Error reading - close connection
			write(2, "Error reading data. Closing connection.\n", 41);
			close(this->_fds[index].fd);
			removeClient(this->_fds, clients, g_num_fds, index);
			return;
		}

		// Null terminate the buffer
		this->buffer[bytes] = '\0';
		
		// Append new data to client's buffer
		this->clients[index]->appendBuffer(std::string(this->buffer, bytes));
		
		// Get the full buffer and process complete messages
		std::string full_Buffer = this->clients[index]->getBuffer();
		size_t pos;
		
		// Process all complete messages (ending with \r\n)
		while((pos = full_Buffer.find("\r\n")) != std::string::npos)
		{
			// Extract complete message
			std::string message = full_Buffer.substr(0, pos);
			
			// Remove processed message from buffer
			full_Buffer = full_Buffer.substr(pos + 2);
			
			// Process the complete message
				// std::cout << "Complete message: [" << message << "]" << std::endl;  // for debug
				// int i = 1;
					int i = 1;
				while (!message.empty())
				{
					// std::cout << "number " << i << "[" << message << "]" << std::endl; // Debug line
					// i++;
					processCommand(index, message);
				}		
		}
		
		// Update client's buffer with remaining unprocessed data
		this->clients[index]->setBuffer(full_Buffer);
	}
}

void Server::start()
{
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
				else if (this->_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
				{
					// Client disconnected or error
					removeClient(this->_fds, clients, g_num_fds, i);
					close(this->_fds[i].fd);
					i--; // Adjust index after removal
				}
			}
		}
	}
	
}