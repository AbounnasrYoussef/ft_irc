#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "fcntl.h"

class Channel;
class Client;

Server::~Server()
{
    close(this->server_Fd);
}

Server::Server(int port, std::string password)
{
    this->port      = port;
    this->password  = password;
    this->server_Fd = -1;
}

void Server::setupSocket()
{
    int pp = 1;
    this->server_Fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_Fd == -1)
    {
        std::cerr << "Error: Failed to create socket" << std::endl;
        exit(1);
    }
    if (fcntl(this->server_Fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "Error: Failed to set server socket to non-blocking" << std::endl;
        close(this->server_Fd);
        exit(1);
    }
    if (setsockopt(this->server_Fd, SOL_SOCKET, SO_REUSEADDR, &pp, sizeof(int)) == -1)
    {
        std::cerr << "Error: Failed setsockopt" << std::endl;
        close(this->server_Fd);
        exit(1);
    }

    struct sockaddr_in address;
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(this->port);

    if (bind(this->server_Fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        std::cerr << "Error: Failed to bind to port " << this->port << std::endl;
        close(this->server_Fd);
        exit(1);
    }
    if (listen(this->server_Fd, 3) == -1)
    {
        std::cerr << "Error: Failed to listen on socket " << this->port << std::endl;
        close(this->server_Fd);
        exit(1);
    }

    struct pollfd serverEntry;
    serverEntry.fd      = this->server_Fd;
    serverEntry.events  = POLLIN;
    serverEntry.revents = 0;
    this->_fds.push_back(serverEntry);
    this->clients.push_back(NULL);
}

void Server::accept_NewClient()
{
    sockaddr_storage client_addr;
    socklen_t        len = sizeof(client_addr);

    int client_fd = accept(this->server_Fd, (sockaddr *)&client_addr, &len);
    if (client_fd == -1)
        return;

    std::string ip = getClientIP(client_addr, len);

    struct pollfd clientEntry;
    clientEntry.fd      = client_fd;
    clientEntry.events  = POLLIN;
    clientEntry.revents = 0;
    this->_fds.push_back(clientEntry);

    Client *newClient = new Client(client_fd);
    newClient->setIP(ip);
    this->clients.push_back(newClient);
}

void Server::handle_ClientData(int index)
{
    if (this->_fds[index].revents & POLLIN)
    {
        int bytes = read(this->_fds[index].fd, this->buffer, 511);

        if (bytes == 0)
        {
            removeClient(this->_fds, clients, index);
            return;
        }
        if (bytes == -1)
        {
            write(2, "Error reading data. Closing connection.\n", 41);
            close(this->_fds[index].fd);
            removeClient(this->_fds, clients, index);
            return;
        }

        this->buffer[bytes] = '\0';
        this->clients[index]->appendBuffer(std::string(this->buffer, bytes));

        std::string full_Buffer = this->clients[index]->getBuffer();
        size_t      pos;

        while ((pos = full_Buffer.find("\r\n")) != std::string::npos)
        {
            std::string message = full_Buffer.substr(0, pos);
            full_Buffer         = full_Buffer.substr(pos + 2);
            while (!message.empty())
                processCommand(index, message);
        }
        this->clients[index]->setBuffer(full_Buffer);
    }
}

void Server::start()
{
    setupSocket();
    while (true)
    {
        int ret = poll(&this->_fds[0], this->_fds.size(), -1);
        if (ret == -1)
            exit(0);
        if (ret > 0)
        {
            if (this->_fds[0].revents & POLLIN)
                accept_NewClient();

            for (int i = 1; i < (int)this->_fds.size(); i++)
            {
                if (this->_fds[i].revents & POLLIN)
                {
                    handle_ClientData(i);
                }
                else if (this->_fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                {
                    removeClient(this->_fds, clients, i);
                    i--;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────
//  Gestion des channels
// ─────────────────────────────────────────────

Channel *Server::findOrCreateChannel(const std::string &name)
{
    if (_channels.find(name) != _channels.end())
        return _channels[name];
    Channel *newChan = new Channel(name);
    _channels[name]  = newChan;
    return newChan;
}

bool Server::findChannel(const std::string &name)
{
    return (_channels.find(name) != _channels.end());
}

// ✅ CORRIGÉ : utilise le vecteur clients (et non _clients qui est vide)
Client *Server::findClient(const std::string &nickname)
{
    for (size_t i = 1; i < this->clients.size(); i++)
    {
        if (this->clients[i] && this->clients[i]->getNickname() == nickname)
            return this->clients[i];
    }
    return NULL;
}
