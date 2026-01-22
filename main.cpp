#include <string>
#include <iostream>
#include <iostream>
#include <unistd.h>

#include <string>        // std::string
#include <netdb.h>       // getnameinfo, NI_MAXHOST
#include <sys/socket.h>  // sockaddr, sockaddr_storage
#include <netinet/in.h>  // sockaddr_in, sockaddr_in6
#include <arpa/inet.h>   // AF_INET, AF_INET6
#include "includes/Server.hpp"

int g_num_fds = 1;

bool ft_isdigit(char *str)
{
	while (*str)
	{
		if (!isdigit(*str))
			return (false);
		str++;
	}

	return ( true );
}

int main(int ac, char **av)
{
	if (ac != 3 || !av[1][0] || !av[2][0])
	{
		std::cerr << "Usage: " << av[0] << " <PORT>"  << " <PASSWORD>" << std::endl;
		return 1;
	}

	if (ft_isdigit(av[1]) == false ||  std::atoi(av[1]) == false)
	{
		std::cerr << "Error: accept only number in port and password" << std::endl;
		return 1;
	}
	int port = std::atoi(av[1]);
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Error: port must be between 1 and 65535" << std::endl;
		return 1;
	}
	
	Server ser(port, av[2]);
	ser.start();

	return 0;
}
