#include <iostream>

int main()
{
	std::string buffer = "NICK alice\r\nPASS secret\r\nUSER";

	// Find first \r\n
	size_t pos = buffer.find("\r\n");  // pos = ???
	// std::cout << pos << 
	// // Extract message
	// 	std::string message = buffer.substr(0, pos);  // me	ssage = ???

	// // Remove from buffer
	// buffer = buffer.substr(pos + 2);  // buffer = ???
	std::cout << buffer[10] << "\n";
	
}