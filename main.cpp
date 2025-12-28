#include <string>
#include <iostream>


bool split(const std::string &s, char delimiter,
           std::string &left, std::string &right)
{
    std::string::size_type pos = s.find(delimiter);

    if (pos == std::string::npos)
        return false;

    left = s.substr(0, pos);
    right = s.substr(pos + 1);

    return !left.empty() && !right.empty();
}

int main()
{
	std::string command;
	std::string argment;
	// command =  ;
	if (split("USER alice 0 * :Alice Smith", ' ', command, argment))
	{
		std::cout << "Command: " << command << std::endl;
		std::cout << "Argument: " << argment << std::endl;
	}
	else
	{
		std::cout << "Error: Invalid command format." << std::endl;
	}
}
