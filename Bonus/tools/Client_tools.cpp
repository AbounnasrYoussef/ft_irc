#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

void removeClient(std::vector<struct pollfd>& fds, std::vector<Client*>& clients, int index)
{
	close(fds[index].fd);
	delete clients[index];
	fds.erase(fds.begin() + index);
	clients.erase(clients.begin() + index);
	throw "Client removed"; // Throw an exception to break out of the calling loop
}

std::string getClientIP(const sockaddr_storage &addr, socklen_t len) // i nedd learn from here this function
{
	char host[NI_MAXHOST];

	if (getnameinfo((const sockaddr *)&addr, len, host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0)
	{
		return std::string(host);
	}
	return std::string();
}

bool user_parsing(const std::string &argument, Client *client) // need learn for this fuction
{
	size_t pos = argument.find(" :");
	if (pos == std::string::npos)
		return false;

	std::string before = argument.substr(0, pos);
	std::string realname = argument.substr(pos + 2);

	if (realname.empty())
		return false;

	std::istringstream iss(before);
	std::string username, hostname, servername;

	if (!(iss >> username >> hostname >> servername))
		return false;

	if (username.empty())
		return false;

	client->setUsername(username);
	client->setRealname(realname);

	return true;
}

bool split(std::string &s, std::string &left, std::string &right)
{
    size_t pos_nl = s.find('\n');
    std::string line;

    if (pos_nl != std::string::npos)
    {
        line = s.substr(0, pos_nl);
        s = s.substr(pos_nl + 1); 
    }
    else
    {
        line = s;
        s = "";
    }

    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    size_t pos_space = line.find(' ');

    if (pos_space == std::string::npos)
    {
        left = line;
        right = "";
        return !left.empty();
    }
    else
    {
        left = line.substr(0, pos_space);
        right = line.substr(pos_space + 1);
        return !left.empty();
    }

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
	// Check length: must be 1-9 characters
	if (_nickname.empty() || _nickname.length() > 9)
		return false;

	// First character must be letter or special (not digit)
	char first = _nickname[0];
	if (!isalpha(first) && first != '-' && first != '[' && first != ']' && 
	    first != '\\' && first != '`' && first != '^' && first != '{' && 
	    first != '}' && first != '|')
		return false;

	// Remaining characters can be letter, digit, or special
	for (size_t i = 1; i < _nickname.length(); ++i)
	{
		char c = _nickname[i];
		if (!isalnum(c) && c != '-' && c != '[' && c != ']' && 
		    c != '\\' && c != '`' && c != '^' && c != '{' && 
		    c != '}' && c != '|')
			return false;
	}

	return true;
}




