#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../includes/Client.hpp"
// ✔️ khaddam logic dyalo s7i7

// ✔️ multi-client b poll

// ❌ khasso client cleanup

// ❌ khasso error handling

// ❌ khasso limits checks

bool Server::isNicknameTaken(std::string nickname)
{
	for (int i = 1; i < this->_numFds; i++)
	{
		if (this->_clients[i]->getNickname() == nickname)
			return true;
	}
	return false;
}
void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index)
{
	// Close the connection
	close(fds[index].fd);
	
	// Delete the Client object
	delete clients[index];
	clients[index] = NULL;
	
	// Shift both arrays
	for (int i = index; i < num_fds - 1; i++) {
		fds[i] = fds[i + 1];
		clients[i] = clients[i + 1];
	}
	
	// Decrease count
	num_fds--;
}

bool split(const std::string &s, char delimiter,
			std::string &left, std::string &right)
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

bool pars_nick(std::string _nickname)
{
	// Nickname must start with a letter or special character
	if (isalpha_string(_nickname))
		return true;

	// // Check each character in the nickname
	// for (size_t i = 1; i < _nickname.length(); ++i)
	// {
	// 	char c = _nickname[i];
	// 	if (!isalnum(c) && !strchr("[]\\`_^{|}-", c))
	// 		return true;
	// }

	return false;
}

void Server::processCommand(Client* client, std::string message)
{
	std::string command;
    std::string argument;

	if (split(message, ' ', command, argument))
	{
	
		if (command == "PASS")
		{
			// if already registered and try again to register 
			if (client->isRegistered())
			{
				sendError(client->get_fd(), "server 462 : You may not reregister\r\n");
				return;
			}

			// PASS already accepted before (even if not registered)
			if (client->isPassOk())
			{
				sendError(client->get_fd(), "server 462 : You may not reregister\r\n");
				return;
			}

			// missing argument
			if (argument.empty())
			{
				sendError(client->get_fd(), "server 461 PASS : Not enough parameters\r\n");
				return;
			}

			// wrong password
			if (argument != this->_password)
			{
				sendError(client->get_fd(), "server 464 : Password incorrect\r\n");
				return;
			}

			// correct password
			client->setPassOk(true);
		}
		else if (command == "NICK")
		{
			if (!client->isPassOk())
			{
				sendError(client->get_fd(), "server 451 : You have not registered\r\n");
				return;
			}
			if (argument.empty())
			{
				sendError(client->get_fd(), "server 431 : No nickname given\r\n");
				return;
			}
			// Check if nickname is already in use
			if (this->isNicknameTaken(argument))
			{
				sendError(client->get_fd(), "server 433 " + argument + " : Nickname is already in use\r\n");
				return ;
			}
			if (this->pars_nick(argument))
			{
				sendError(client->get_fd(), "server 432 " + argument + " : Erroneous nickname\r\n");
				return ;
			}
			client->setNickname(argument);
		}
		else if (command == "USER")
		{
			// Handle USER command
			// Step 3 - client sends USER
			// 	isPassOk()      = true
			// 	isRegistered() = true
		}
		else
		{
			// Unknown command
		}
	}
	else
	{
		client->setPassOk(false);
		// Invalid command format
	}
			// check user and pass and niclk are set
			// if (client->isRegistered() && client->getUsername().empty() == false)
			// {
			// 	sendError(client->get_fd(), "server 001 " + argument + " : Welcome to the Internet Relay Network\r\n");
			// }

}

int main() {
	// 1. Create server socket
	// int MAX_CLIENTS = 100;
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // buffering tcp create

	// 2. Bind to port 
	struct sockaddr_in address;   /// socket(ip:port) for this process 
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(6667);
	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	
	// 3. Start listening
	listen(server_fd, 3); // am ready to accept connections (work) (10.50.20.7:6667)
	
	// 4. Set up poll
	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	int num_fds = 1;
	
	// 5. Main loop
	Client* clients[MAX_CLIENTS];
	while (true)
	{
		// ===============================================================
		// Wait for activity on any file descriptor
		int ret = poll(fds, num_fds, -1);  // -1 = wait forever
		
		if (ret == -1) {
			// Error - clean up and exit
			exit(0);
		}
		
		if (ret > 0) {  // Something happened!
			// ==============================
			// Check if someone wants to connect
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(fds[0].fd, NULL, NULL); // Accept new connection or client
				char ip[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip));
				// Add new client to our monitoring list
				fds[num_fds].fd = client_fd;
				fds[num_fds].events = POLLIN;
				clients[num_fds] = new Client(client_fd);
				num_fds++; // add to array
				// crate new client in arr of class
			}
			
			// ==============================
			// Check all connected clients for messages
			for (int i = 1; i < num_fds; i++) {
				if (fds[i].revents & POLLIN) {
					char buffer[512];
					int bytes = read(fds[i].fd, buffer, 512);

					if (bytes == 0)
					{
						// Client disconnected
						
						write(fds[i].fd, "Client is diconnected Goodbye!\n", 9);
						removeClient(fds, clients, num_fds, i);
						continue;
					}

					if (bytes == -1)
					{
						// Error reading - close connection
						write(2, "Error reading data. Closing connection.\n", 41);
						close(fds[i].fd);
						// TODO: Remove from array
					}
					else
					{
						buffer[bytes] = '\0';
						// clients[i]->appendBuffer(buffer);
						clients[i]->appendBuffer(std::string(buffer));
						std::string full_Buffer = clients[i]->getBuffer();
						size_t pos;
						while((pos = full_Buffer.find("\r\n")) != std::string::npos)
						{
							std::string message = full_Buffer.substr(0, pos);
							full_Buffer = full_Buffer.substr(pos + 2);
							std::cout << "Complete message: [" << message << "]" << std::endl;  // for debug
							
							 // handl commands and parse(PASS, NICK ,USER)

							write(fds[i].fd, "Message received: ", 18);
						}
						// if (massage_complet(clients[i]->getBuffer()))
						// {
						// 	// message commplet 
						// }
						// parse message
						// Process message
						write(fds[i].fd, "Hello from server!\n", 19);
					}
				}
			}
		}
		// ===============================================================
	}
	
	return 0;
}


