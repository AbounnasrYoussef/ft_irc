#include "../includes/Server.hpp"


void ft_toupper(std::string &str)
{
	size_t i = 0;
	size_t len = str.length();
	
	while (i < len)
	{
		str[i] = std::toupper(str[i]);
		i++;
	}

}
