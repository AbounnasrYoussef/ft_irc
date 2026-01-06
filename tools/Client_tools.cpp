#include "../includes/Client.hpp"

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

std::string getClientIP(const sockaddr_storage &addr, socklen_t len) // i nedd learn from here this function 
{
	char host[NI_MAXHOST];

	if (getnameinfo((const sockaddr*)&addr, len, host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0)
	{
		return std::string(host);
	}
	return std::string();
}

bool user_parsing(const std::string& argument, Client* client) // need learn for this fuction
{
	// 1) realname must start with ':'
	size_t colonPos = argument.find(" :");
	if (colonPos == std::string::npos)
		return false;

	// 2) split into "before :" and "realname"
	std::string before = argument.substr(0, colonPos);
	std::string realname = argument.substr(colonPos + 2);

	if (realname.empty())
		return false;

	// 3) split the part before ':' into tokens
	std::istringstream iss(before);
	std::string username, hostname, servername;

	if (!(iss >> username >> hostname >> servername))
		return false;

	// 4) store values
	client->setUsername(username);
	client->setRealname(realname);

	return true;
}

bool split(std::string &s, char delimiter, std::string &left, std::string &right)
{
	size_t pos = s.find('\n');
	size_t pos_space = s.find(' ');
	std::string tmp;

	if (pos == std::string::npos)
	{
		if (pos_space == std::string::npos)
		{
			return false;
		}
		else
		{
			tmp = s;
			tmp = s.substr(0, pos);
			std::string copy = s.substr(pos + 1);
			s = copy;
			left = tmp.substr(0, pos_space);
			right = tmp.substr(pos_space + 1);
			s = "";
			return (!left.empty() && !right.empty());
		}
	}

	tmp = s;
	tmp = s.substr(0, pos);
	std::string copy = s.substr(pos + 1);
	s = copy;
	pos = tmp.find(' ');

	if (pos == std::string::npos)
		return false;

	left = tmp.substr(0, pos);
	right = tmp.substr(pos + 1);
	return (!left.empty() && !right.empty());
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
	if (isalpha_string(_nickname))
		return true;

	return false;
}

