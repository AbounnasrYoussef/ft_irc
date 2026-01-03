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
	this->password = password;
	this->server_Fd = -1;
}

void Server::setupSocket()
{
	// this->_
	// this->_serverFd
	this->server_Fd = socket(AF_INET, SOCK_STREAM, 0); // buffering tcp create
	if (this->server_Fd == -1)
	{
		perror("Socket creation failed");
		exit(1); // need to handle error properly clean up
	}
	if (fcntl(this->server_Fd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Failed to set non-blocking mode");
		exit(1); // need to handle error properly clean up
	}
	// 2. Bind to port 
	struct sockaddr_in address;   /// socket(ip:port) for this process 
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(this->port);
	if (bind(this->server_Fd, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		perror("Bind failed");
		exit(1); // need to handle error properly clean up
	}

	// 3. Start listening
	listen(this->server_Fd, 3); // am ready to accept connections (work) (10.50.20.7:6667)
	// this->_fds[MAX_CLIENTS];
	this->_fds[0].fd = this->server_Fd;
	this->_fds[0].events = POLLIN;
}


void Server::accept_NewClient()
{
	sockaddr_storage client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(this->server_Fd, (sockaddr*)&client_addr, &len);
	fcntl(client_fd, F_SETFL, O_NONBLOCK); // set non blocking mode
	std::string ip = getClientIP(client_addr, len);
	// Add new client to our monitoring list

	this->_fds[g_num_fds].fd = client_fd;
	this->_fds[g_num_fds].events = POLLIN;
	this->clients[g_num_fds] = new Client(client_fd);
	this->clients[g_num_fds]->setIP(ip); // set ip of client
	
	g_num_fds++; // add to array
}


// void Server::handle_ClientData(int index)
// {
// 	//  Process client messages

// 	std::cout << this->clients[index]->getIP()  << std::endl; // for debug
// 	// for (int i = 1; i < g_num_fds; i++)
// 	// {
// 		chae
// 		if (this->_fds[i].revents & POLLIN)
// 		{
// 			// char buffer[512];
// 			int bytes = read(this->_fds[i].fd, this->buffer, 512);

// 			if (bytes == 0)
// 			{
// 				// Client disconnected
// 				write(2, "Client is diconnected Goodbye!\n", 32);
// 				removeClient(this->_fds, clients, g_num_fds, i);
// 			}

// 			if (bytes == -1)
// 			{
// 				// Error reading - close connection
// 				std::cerr << "read from client failed" << std::endl;
// 				close(this->_fds[i].fd);
// 				// clean up client
// 			}

// 			// else
// 			// {
// 			// 	this->buffer[bytes] = '\0';
// 			// 	// this->clients[i]->appendBuffer(buffer);
// 			// 	this->clients[i]->appendBuffer(std::string(this->buffer));
// 			// 	std::string full_Buffer = this->clients[i]->getBuffer(); // 
// 			// 	size_t pos;
// 			// 	std::cout << this->buffer << std::endl; // for debug
// 			// 	while((pos = full_Buffer.find("\r\n")) != std::string::npos)
// 			// 	{
// 			// 		std::string message = full_Buffer.substr(0, pos);
// 			// 		full_Buffer = full_Buffer.substr(pos + 2);
// 			// 	}
// 			// 	processCommand(i, full_Buffer); // handle after is commenands success
// 			// 	// Successfully processed command
// 			// 	// if (massage_complet(this->clients[i]->getBuffer()))
// 			// 	// {
// 			// 	// 	// message commplet 
// 			// 	// }

// 			// }
// 		}
// 	// }
// }

void Server::handle_ClientData(int index)
{
    char buf[512];
    int bytes = read(this->_fds[index].fd, buf, sizeof(buf));

    // 1) Client disconnected
    if (bytes == 0)
    {
        std::cerr << "Client disconnected: "
                  << this->clients[index]->getIP() << std::endl;
        removeClient(this->_fds, this->clients, g_num_fds, index);
        return;
    }

    // 2) Read error
    if (bytes < 0)
    {
        // Non-blocking: EAGAIN / EWOULDBLOCK are NOT errors
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "Read error from client: "
                      << this->clients[index]->getIP() << std::endl;
            removeClient(this->_fds, this->clients, g_num_fds, index);
        }
        return;
    }

    // 3) Append received data to client buffer
    buf[bytes] = '\0';
    this->clients[index]->appendBuffer(std::string(buf));

    // 4) Process complete commands (\r\n terminated)
    std::string& clientBuffer = this->clients[index]->getMutableBuffer();
    size_t pos;

    while ((pos = clientBuffer.find("\r\n")) != std::string::npos)
    {
        std::string command = clientBuffer.substr(0, pos);
        clientBuffer.erase(0, pos + 2); // remove processed command

        // Handle ONE command at a time
        processCommand(index, command);
    }
}


void Server::start()
{
	setupSocket();
	while(g_running)
	{
		int ret = poll(this->_fds, g_num_fds, -1);  // -1 = wait forever
		if (ret == -1)
		{
			// Error - clean up and exit
			perror("poll failed");
			// exit(0);
			return;
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
	if (g_running == false)
	{
		// Clean up before exiting
		close(this->server_Fd);
		write(2, "Server test dowrn \n", 20);
		for (int i = 0; i < g_num_fds; i++)
		{
			close(this->_fds[i].fd);
			delete this->clients[i];
		}
	}
	
}