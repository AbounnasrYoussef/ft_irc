#include <iostream>
#include "Client.hpp"
#include <unistd.h> // for close()
#include <poll.h>

void removeClient(struct pollfd fds[], Client* clients[], int& num_fds, int index)
{
	// Close the connection
	close(fds[index].fd);
	
	// Delete the Client object
	delete clients[index];
	clients[index] = NULL;
	
	// Shift both arrays
	for (int i = index; i < num_fds - 1; i++) {
		fds[i] = fds[i + 1];
		clients[i] = clients[i + 1];
	}
	
	// Decrease count
	num_fds--;
}
