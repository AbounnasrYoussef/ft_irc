#include "../includes/Client.hpp"

std::string getClientIP(const sockaddr_storage &addr, socklen_t len) // i nedd learn from here this function 
{
	char host[NI_MAXHOST];

	if (getnameinfo((const sockaddr*)&addr, len, host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0)
	{
		return std::string(host);
	}
	return std::string();
}
