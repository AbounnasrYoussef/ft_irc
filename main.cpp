#include <string>
#include <iostream>

bool isalpha_string(std::string str)
{
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (!isalpha(str[i]))
			return false;
	}
	return true;
}
#include <iostream>
#include <unistd.h>

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6669);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));

    std::cout << "Client IP: " << ip << std::endl;
}


