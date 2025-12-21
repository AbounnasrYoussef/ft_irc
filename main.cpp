#include <arpa/inet.h>
#include <iostream>

int main() {

	// socket   ==> create socket 
	// bind to a port  // htons() >> little endian and big endian
	// lisen()  ==> katwajed server bach y9bel clients
	// accept() ==> accept clients

	// poll() ==> to handl multiple clients  //use struct pollfd{int fd; short events; short revents;}



	// struct pollfd fds[MAX_CLIENTS];

	struct sockaddr_in add;
	add.
	// struct pollfd fds[MAX_CLIENTS];
	// fds[0].fd = server_fd;
	// fds[0].events = POLLIN;
	// int num_fds = 1;
	// int f = socket(AF_INET, SOCK_STREAM, 0); // for what use 0 ?
	// while (true) {
	// 	int ret = poll(fds, num_fds, -1);  // -1 = wait forever
		
	// 	if (ret == -1)
	// 	{
	// 		// claen all leaks
	// 		exit(0);
	// 	}
	// 	if (ret > 0) {  // Something happened!
			
	// 		// Check server socket
	// 		if (fds[0].revents & POLLIN) {
	// 			// What should you do here?
	// 			int client_fd = accept(fds[0].fd, NULL,NULL);
	// 			fds[num_fds].fd = client_fd;
	// 			fds[num_fds].events = POLLIN;
	// 			num_fds++;
	// 			// Hint: Something is trying to connect!
	// 		}
			
	// 		// Check all clients
	// 		for (int i = 1; i < num_fds; i++) {
	// 			if (fds[i].revents & POLLIN) {
	// 				char buffer[1024];
	// 				read(fds[i].fd, buffer, 1024);
	// 				write(fds[i].fd, "Hello from server!\n", 19);
	// 				// What should you do here?
	// 				// Hint: This client sent a message!
	// 				close(fds[i].fd);
	// 			}
	// 		}
	// 	}
	// }
}