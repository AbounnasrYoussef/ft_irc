#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <ctime>

class Client;
class Channel;

extern int g_num_fds;

void sendError(int fd, const std::string& msg);
void ft_toupper(std::string &str);

class Server {
private:
    int         _serverFd;
    std::string _password;
    std::map<std::string, Channel*> _channels;

public:
    int                       port;
    std::vector<struct pollfd> _fds;
    std::vector<Client*>      clients;

    Server(int port, std::string password);
    ~Server();

    void start();

    // Bot
    int  check_client_is_live(int index, std::string argument);
    void bot(std::string &message, std::string command, std::string argument, int index);

    // IRC commands
    void    handel_Invite(std::string &command, std::string &argument, int index);
    void    handel_Join(std::string &command, std::string &argument, int index);
    void    handle_kick(int kicker_index, const std::string& argument);
    void    handle_mode(int setter_index, const std::string& argument);
    Client* get_client_by_nickname(const std::string& nickname);
    void    handle_privmsg(int sender_index, const std::string& argument);
    void    handel_Topic(std::string &command, std::string &argument, int index);

    // Channel helpers
    Channel* findChannel(const std::string& name);
    Channel* findOrCreateChannel(const std::string& name);
    Client*  findClient(const std::string& nickname);
    Channel* get_channel(const std::string& name);
    Channel* create_channel(const std::string& name);
    void     delete_channel(Channel* channel);
};

#include "Client.hpp"
#include "Channel.hpp"

#endif
