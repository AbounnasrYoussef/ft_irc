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
bool g_running = true;

void f()
{
    system("leaks ircserv");
}

void signalHandler(int signum)
{
    if (signum == SIGINT)  // Ctrl+C
    {
        std::cout << "\nShutting down server..." << std::endl;
        g_running = false;  // Tell main loop to exit
    }
}
int main(int ac, char **av)
{

	signal(SIGINT, signalHandler);

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
	// atexit(f);
	// while (1)
	// {
	// 	/* code */
	// }
	
	return 0;
}
