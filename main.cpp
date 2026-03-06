#include <string>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <netdb.h>       // getnameinfo, NI_MAXHOST
#include <sys/socket.h>  // sockaddr, sockaddr_storage
#include <netinet/in.h>  // sockaddr_in, sockaddr_in6
#include <arpa/inet.h>   // AF_INET, AF_INET6
#include "includes/Server.hpp"

// OLD CODE - CRITICAL: No validation of port or password
// int main(int ac, char **av)
// {
// 	if (ac != 3)
// 	{
// 		std::cerr << "Usage: " << av[0] << " <PORT>"  << " <PASSWORD>" << std::endl;
// 		return 1;
// 	}
// 	Server ser(std::atoi(av[1]), av[2]);
// 	ser.start();
// 	return 0;
// }
bool g_running = true; 

#include <signal.h>
// NEW CODE - Proper validation of port and password
void f()
{
    system("leaks ircserv");
}




extern "C" void server_signal(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
        g_running = 0;
		// exit(0);
	std::cout << "\nShutting down server..." << std::endl;
}

void setup_server_signals()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = server_signal;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: " << av[0] << " <PORT> <PASSWORD>" << std::endl;
		return 1;
	}
	setup_server_signals();
	// Validate port number
	char *endptr;
	long port = std::strtol(av[1], &endptr, 10);
	
	// Check if conversion was successful and port is in valid range
	if (*endptr != '\0' || port <= 0 || port > 65535)
	{
		std::cerr << "Error: Invalid port number (must be 1-65535)" << std::endl;
		return 1;
	}
	
	// Warn about privileged ports
	if (port < 1024)
	{
		std::cerr << "Warning: Port " << port << " may require root privileges" << std::endl;
	}
	
	// Validate password
	std::string password = av[2];
	if (password.empty())
	{
		std::cerr << "Error: Password cannot be empty" << std::endl;
		return 1;
	}
	
	// Create and start server
	Server ser(port, password);
	ser.start();
	atexit(f);

	return 0;
}



