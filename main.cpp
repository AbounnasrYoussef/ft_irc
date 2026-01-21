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

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: " << av[0] << " <PORT>"  << " <PASSWORD>" << std::endl;
		return 1;
	}
	// if (std::atoi(av[1]) <= 0 || !isalpha_string(av[2]))
	// {
	// 	std::cerr << "Error: Invalid port number." << std::endl;
	// 	return 1;
	// }

	Server ser(std::atoi(av[1]), av[2]);
	ser.start();

	return 0;
}



