#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>

// ✔️ khaddam logic dyalo s7i7

// ✔️ multi-client b poll

// ❌ khasso client cleanup

// ❌ khasso error handling

// ❌ khasso limits checks

int main() {
	// 1. Create server socket
	int MAX_CLIENTS = 100;
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // buffering tcp create

	// 2. Bind to port 
	struct sockaddr_in address;   /// socket(ip:port) for this process 
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(6667);
	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	
	// 3. Start listening
	listen(server_fd, 3); // am ready to accept connections (work) (10.50.20.7:6667)
	
	// 4. Set up poll
	struct pollfd fds[MAX_CLIENTS];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	int num_fds = 1;
	
	// 5. Main loop
	while (true)
	{
		// ===============================================================
		// Wait for activity on any file descriptor
		int ret = poll(fds, num_fds, -1);  // -1 = wait forever
		
		if (ret == -1) {
			// Error - clean up and exit
			exit(0);
		}
		
		if (ret > 0) {  // Something happened!
			// ==============================
			// Check if someone wants to connect
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(fds[0].fd, NULL, NULL); // Accept new connection or client
				
				// Add new client to our monitoring list
				fds[num_fds].fd = client_fd;
				fds[num_fds].events = POLLIN;
				num_fds++; // add to array
			}
			
			// ==============================
			// Check all connected clients for messages
			for (int i = 1; i < num_fds; i++) {
				if (fds[i].revents & POLLIN) {
					char buffer[1024];
					int bytes = read(fds[i].fd, buffer, 1024);
					
					if (bytes <= 0) {
						// Client disconnected
						close(fds[i].fd);
						// TODO: Remove from array
					} else {
						// Process message
						write(fds[i].fd, "Hello from server!\n", 19);
					}
				}
			}
		}
		// ===============================================================
	}
	
	return 0;
}