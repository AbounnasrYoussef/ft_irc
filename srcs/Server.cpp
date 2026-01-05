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
	//  Process client messages for the specific client at index

	if (this->_fds[index].revents & POLLIN)
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
			if (!message.empty())
			{
				// std::cout << "Complete message: [" << message << "]" << std::endl;  // for debug
				int i = 1;
				while (!message.empty())
				{
					// std::cout << "number " << i << "[" << message << "]" << std::endl; // Debug line
					processCommand(index, message);

				}
				
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