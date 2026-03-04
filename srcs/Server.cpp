#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

int g_num_fds = 1;

void sendError(int fd, const std::string& msg)
{
    send(fd, msg.c_str(), msg.size(), 0);
}

Server::Server(int port, std::string password)
    : _serverFd(-1), _password(password), port(port)
{
}

Server::~Server()
{
    for (size_t i = 1; i < clients.size(); i++) {
        if (clients[i]) {
            close(clients[i]->get_fd());
            delete clients[i];
        }
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    if (_serverFd != -1)
        close(_serverFd);
}

Channel* Server::findChannel(const std::string& name)
{
    return get_channel(name);
}

Channel* Server::findOrCreateChannel(const std::string& name)
{
    Channel* ch = get_channel(name);
    if (ch)
        return ch;
    return create_channel(name);
}

Client* Server::findClient(const std::string& nickname)
{
    for (size_t i = 1; i < clients.size(); i++) {
        if (clients[i] && clients[i]->getNickname() == nickname)
            return clients[i];
    }
    return NULL;
}

static bool parse_irc_line(const std::string& line, std::string& cmd, std::string& arg)
{
    size_t i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\r' || line[i] == '\n')) i++;
    if (i == line.size()) return false;

    size_t start = i;
    while (i < line.size() && line[i] != ' ' && line[i] != '\r' && line[i] != '\n') i++;
    cmd = line.substr(start, i - start);

    while (i < line.size() && line[i] == ' ') i++;
    size_t arg_end = line.size();
    while (arg_end > i && (line[arg_end - 1] == '\r' || line[arg_end - 1] == '\n')) arg_end--;
    arg = (i < arg_end) ? line.substr(i, arg_end - i) : "";

    return !cmd.empty();
}

void Server::start()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0) {
        std::cerr << "Error: socket() failed" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_serverFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: bind() failed on port " << port << std::endl;
        close(_serverFd);
        return;
    }

    if (listen(_serverFd, 10) < 0) {
        std::cerr << "Error: listen() failed" << std::endl;
        close(_serverFd);
        return;
    }

    std::cout << "ft_irc server started on port " << port << std::endl;

    struct pollfd server_pfd;
    server_pfd.fd = _serverFd;
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;
    _fds.push_back(server_pfd);
    clients.push_back(NULL);
    g_num_fds = 1;

    while (true) {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret < 0) {
            std::cerr << "Error: poll() failed" << std::endl;
            break;
        }

        // Accept new connections
        if (_fds[0].revents & POLLIN) {
            struct sockaddr_storage caddr;
            socklen_t clen = sizeof(caddr);
            int cfd = accept(_serverFd, (struct sockaddr*)&caddr, &clen);
            if (cfd >= 0) {
                fcntl(cfd, F_SETFL, O_NONBLOCK);
                struct pollfd pfd;
                pfd.fd = cfd;
                pfd.events = POLLIN;
                pfd.revents = 0;
                _fds.push_back(pfd);
                Client* c = new Client(cfd);
                c->setIP(getClientIP(caddr, clen));
                clients.push_back(c);
                g_num_fds = (int)_fds.size();
                std::cout << "Client connected: fd=" << cfd << std::endl;
            }
        }

        // Handle existing clients
        for (size_t i = 1; i < _fds.size(); i++) {
            if (!(_fds[i].revents & POLLIN))
                continue;

            char buf[512];
            memset(buf, 0, sizeof(buf));
            int bytes = recv(_fds[i].fd, buf, sizeof(buf) - 1, 0);

            if (bytes <= 0) {
                std::cout << "Client disconnected: fd=" << _fds[i].fd << std::endl;
                close(_fds[i].fd);
                delete clients[i];
                clients.erase(clients.begin() + i);
                _fds.erase(_fds.begin() + i);
                g_num_fds = (int)_fds.size();
                i--;
                continue;
            }

            clients[i]->appendBuffer(std::string(buf, bytes));

            std::string working = clients[i]->getBuffer();
            clients[i]->clearBuffer();

            bool removed = false;
            size_t pos;
            while ((pos = working.find('\n')) != std::string::npos) {
                std::string line = working.substr(0, pos + 1);
                working = working.substr(pos + 1);

                std::string cmd, arg;
                if (!parse_irc_line(line, cmd, arg))
                    continue;

                ft_toupper(cmd);

                if (!cmd.empty() && cmd[0] == '!') {
                    std::string dummy;
                    bot(dummy, cmd, arg, i);
                    continue;
                }

                if (cmd == "PASS") {
                    if (!clients[i]->isPassOk()) {
                        if (arg == _password)
                            clients[i]->setPassOk(true);
                        else
                            sendError(_fds[i].fd, ":server 464 * :Password incorrect\r\n");
                    }
                } else if (cmd == "NICK") {
                    if (arg.empty()) {
                        sendError(_fds[i].fd, ":server 431 * :No nickname given\r\n");
                    } else if (!pars_nick(arg)) {
                        sendError(_fds[i].fd, ":server 432 * " + arg + " :Erroneous nickname\r\n");
                    } else {
                        bool taken = false;
                        for (size_t j = 1; j < clients.size(); j++) {
                            if (j != i && clients[j] && clients[j]->getNickname() == arg) {
                                taken = true;
                                break;
                            }
                        }
                        if (taken)
                            sendError(_fds[i].fd, ":server 433 * " + arg + " :Nickname is already in use\r\n");
                        else
                            clients[i]->setNickname(arg);
                    }
                } else if (cmd == "USER") {
                    if (!user_parsing(arg, clients[i]))
                        sendError(_fds[i].fd, ":server 461 * USER :Not enough parameters\r\n");
                } else if (cmd == "QUIT") {
                    sendError(_fds[i].fd, "ERROR :Closing Link\r\n");
                    close(_fds[i].fd);
                    delete clients[i];
                    clients.erase(clients.begin() + i);
                    _fds.erase(_fds.begin() + i);
                    g_num_fds = (int)_fds.size();
                    removed = true;
                    i--;
                    break;
                } else if (cmd == "PING") {
                    std::string pong = ":server PONG server :" + arg + "\r\n";
                    send(_fds[i].fd, pong.c_str(), pong.size(), 0);
                } else if (!clients[i]->isRegistered()) {
                    sendError(_fds[i].fd, ":server 451 * :You have not registered\r\n");
                } else if (cmd == "JOIN") {
                    handel_Join(cmd, arg, i);
                } else if (cmd == "PRIVMSG") {
                    handle_privmsg(i, arg);
                } else if (cmd == "KICK") {
                    handle_kick(i, arg);
                } else if (cmd == "MODE") {
                    handle_mode(i, arg);
                } else if (cmd == "TOPIC") {
                    handel_Topic(cmd, arg, i);
                } else if (cmd == "INVITE") {
                    handel_Invite(cmd, arg, i);
                } else {
                    sendError(_fds[i].fd, ":server 421 * " + cmd + " :Unknown command\r\n");
                }

                // Send welcome after first complete registration
                if (!removed && clients[i]->isRegistered() && !clients[i]->isWelcomeSent()) {
                    clients[i]->setWelcomeSent(true);
                    std::string nick = clients[i]->getNickname();
                    std::string w = ":server 001 " + nick + " :Welcome to ft_irc, " + nick + "!\r\n";
                    send(_fds[i].fd, w.c_str(), w.size(), 0);
                }
            }

            if (!removed && !working.empty())
                clients[i]->appendBuffer(working);
        }
    }
}
