#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

// void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index)
// {
// 	close(fds[index].fd);
// 	delete clients[index];
// 	clients[index] = NULL;
// 	for (int i = index; i < num_fds - 1; i++) {
// 		fds[i] = fds[i + 1];
// 		clients[i] = clients[i + 1];
// 	}
// 	num_fds--;
// }
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

// OLD CODE - Missing empty username validation
// bool user_parsing(const std::string &argument, Client *client)
// {
// 	size_t colonPos = argument.find(" :");
// 	if (colonPos == std::string::npos)
// 		return false;
// 	std::string before = argument.substr(0, colonPos);
// 	std::string realname = argument.substr(colonPos + 2);
// 	if (realname.empty())
// 		return false;
// 	std::istringstream iss(before);
// 	std::string username, hostname, servername;
// 	if (!(iss >> username >> hostname >> servername))
// 		return false;
// 	client->setUsername(username);
// 	client->setRealname(realname);
// 	return true;
// }

// NEW CODE - Added empty username validation
bool user_parsing(const std::string &argument, Client *client) // need learn for this fuction
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

	// 4) Validate username is not empty (NEW)
	if (username.empty())
		return false;

	// 5) store values
	client->setUsername(username);
	client->setRealname(realname);

	return true;
}


// bool split(std::string &s, char delimiter, std::string &left, std::string &right)
// {
// 	size_t pos = s.find('\n');
// 	size_t pos_space = s.find(' ');
// 	std::string tmp;

// 	if (pos == std::string::npos)
// 	{
// 		if (pos_space == std::string::npos)
// 		{
// 			return false;
// 		}
// 		else
// 		{
// 			tmp = s;
// 			tmp = s.substr(0, pos);
// 			std::string copy = s.substr(pos + 1);
// 			s = copy;
// 			left = tmp.substr(0, pos_space);
// 			right = tmp.substr(pos_space + 1);
// 			s = "";
// 			return (!left.empty() && !right.empty());
// 		}
// 	}

// 	tmp = s;
// 	tmp = s.substr(0, pos);
// 	std::string copy = s.substr(pos + 1);
// 	s = copy;
// 	pos = tmp.find(' ');

// 	if (pos == std::string::npos)
// 		return false;

// 	left = tmp.substr(0, pos);
// 	right = tmp.substr(pos + 1);
// 	return (!left.empty() && !right.empty());
// }

bool split(std::string &s, char delimiter, std::string &left, std::string &right)
{
	(void)delimiter;
    size_t pos_nl = s.find('\n'); // نبحث عن نهاية السطر أولاً
    std::string line;

    if (pos_nl != std::string::npos)
    {
        line = s.substr(0, pos_nl); // نأخذ السطر حتى \n
        s = s.substr(pos_nl + 1);    // نحذف السطر من الرسالة الأصلية
    }
    else
    {
        line = s;
        s = ""; // لا يوجد \n — نستهلك كل الرسالة
    }

    // الآن ننظف line من \r إن وجد
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    // الآن نبحث عن أول مسافة
    size_t pos_space = line.find(' ');

    if (pos_space == std::string::npos)
    {
        // لا توجد مسافة — إذًا الأمر بدون معطى
        left = line;          // ← كامل السطر هو الأمر
        right = "";              // ← لا يوجد معطى
        return !left.empty(); // نرجع true طالما يوجد أمر
    }
    else
    {
        // يوجد مسافة — نقسم
        left = line.substr(0, pos_space);
        right = line.substr(pos_space + 1);
        return !left.empty();
    }

}
// bool split(std::string &s, char delimiter, std::string &left, std::string &right)
// {
// 	size_t pos = s.find('\n');
// 	size_t pos_space = s.find(' ');
// 	std::string tmp;

// 	if (pos == std::string::npos)
// 	{
// 		if (pos_space == std::string::npos)
// 		{
// 			return false;
// 		}
// 		else
// 		{
// 			tmp = s;
// 			tmp = s.substr(0, pos);
// 			std::string copy = s.substr(pos + 1);
// 			s = copy;
// 			left = tmp.substr(0, pos_space);
// 			right = tmp.substr(pos_space + 1);
// 			s = "";
// 			return (!left.empty() && !right.empty());
// 		}
// 	}

// 	tmp = s;
// 	tmp = s.substr(0, pos);
// 	std::string copy = s.substr(pos + 1);
// 	s = copy;
// 	pos = tmp.find(' ');

// 	if (pos == std::string::npos)
// 		return false;

// 	left = tmp.substr(0, pos);
// 	right = tmp.substr(pos + 1);
// 	return (!left.empty() && !right.empty());
// }

bool isalpha_string(std::string str)
{
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (!isalpha(str[i]))
			return false;
	}
	return true;
}

// OLD CODE - Too restrictive, only accepts alphabetic characters
// bool pars_nick(std::string _nickname)
// {
// 	if (isalpha_string(_nickname))
// 		return true;
// 	return false;
// }

// NEW CODE - RFC 1459 compliant nickname validation
// RFC 1459: <nick> ::= <letter> { <letter> | <number> | <special> }
// <special> ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}' | '|'
// Nickname must be 1-9 characters
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




