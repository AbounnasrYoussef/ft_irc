#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include "includes/Client.hpp"
// ✔️ khaddam logic dyalo s7i7

// ✔️ multi-client b poll

// ❌ khasso client cleanup

// ❌ khasso error handling

// ❌ khasso limits checks

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

void Server::processCommand(Client* client, std::string message)
{
	std::string command;
    std::string argument;

	if (split(message, ' ', command, argument))
	{
		if (command == "PASS")
		{
			// Handle PASS command
			if (client->isRegistered())
		}
		else if (command == "NICK")
		{
			// Handle NICK command
		}
		else if (command == "USER")
		{
			// Handle USER command
		}
		else
		{
			// Unknown command
		}
	}
	else
	{
		// Invalid command format
	}

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
					buffer[bytes] = '\0';

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


