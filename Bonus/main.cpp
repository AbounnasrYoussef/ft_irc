#include "includes/Server.hpp"

bool g_running = true; 

#include <signal.h>

void server_signal(int sig)
{
	(void)sig;
	g_running = 0;
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
	sigaction(SIGQUIT, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: " << av[0] << " <PORT>"  << " <PASSWORD>" << std::endl;
		return 1;
	}
	int port = std::atoi(av[1]);
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Error: Invalid port number (must be 1-65535)" << std::endl;
		return 1;
	}
	try
	{
		setup_server_signals();
		std::string password = av[2];
		if (password.empty())
		{
			std::cerr << "Error: Password cannot be empty" << std::endl;
			return 1;
		}
		Server ser(port, password);
		ser.start();
	}
	catch(...)
	{
		std::cerr << "SERVER IS DOWN!"<< '\n';
	}
	return 0;
}
