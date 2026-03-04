#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include <sstream>

std::string getClientIP(const sockaddr_storage &addr, socklen_t len)
{
    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&addr, len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
        return std::string(host);
    return "unknown";
}

void removeClient(std::vector<struct pollfd>& fds, std::vector<Client*>& clients, int index)
{
    if (index < 0 || (size_t)index >= clients.size())
        return;
    close(fds[index].fd);
    delete clients[index];
    clients.erase(clients.begin() + index);
    fds.erase(fds.begin() + index);
}

void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index)
{
    close(fds[index].fd);
    delete clients[index];
    for (int i = index; i < num_fds - 1; i++) {
        fds[i] = fds[i + 1];
        clients[i] = clients[i + 1];
    }
    num_fds--;
}

bool pars_nick(std::string nick)
{
    if (nick.empty() || nick.size() > 9)
        return false;
    if (!isalpha(nick[0]) && nick[0] != '_')
        return false;
    for (size_t i = 1; i < nick.size(); i++) {
        char c = nick[i];
        if (!isalnum(c) && c != '_' && c != '-')
            return false;
    }
    return true;
}

bool split(std::string &s, char delimiter, std::string &left, std::string &right)
{
    size_t pos = s.find(delimiter);
    if (pos == std::string::npos) {
        left = s;
        right = "";
        return true;
    }
    left = s.substr(0, pos);
    right = s.substr(pos + 1);
    return true;
}

bool user_parsing(const std::string& argument, Client* client)
{
    std::istringstream iss(argument);
    std::string username, hostname, servername, realname;
    if (!(iss >> username >> hostname >> servername))
        return false;
    std::getline(iss, realname);
    if (!realname.empty() && realname[0] == ' ')
        realname = realname.substr(1);
    if (!realname.empty() && realname[0] == ':')
        realname = realname.substr(1);
    if (username.empty())
        return false;
    client->setUsername(username);
    client->setRealname(realname);
    return true;
}
