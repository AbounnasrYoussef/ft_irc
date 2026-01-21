#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

bool check_bot_command(const std::string& command)
{
	return (command == "!HELP" || command == "!TIME" || command == "!DATE" ||
			command == "!USER" || command == "!SERVER" || command == "!ABOUT" ||
			command == "!RULES" || command == "!BATTLE");
}

int Server::check_client_is_live(int index, std::string aragument)
{
	for (size_t i = 1; i < g_num_fds; i++)
	{
		if (this->clients[i]->getNickname() == aragument)
			return i;
	}
	sendError(this->_fds[index].fd, "ERROR : No such this nick \"" + aragument + "\"\r\n");
	return -1;
}

void Server::bot(std::string &message, std::string command, std::string argument, int index)
{
	if (command == "!HELP")
	{
		std::string pass = "Password only numbers like 123"; // Example password
		std::string nicke = "Nickname only letters like JohnDoe"; // Example nickname
		std::string user = "user only letters and use this syntax <username> <hostname> <servername> <realname>\n"; // Example user
	}
	else if (command == "!USER")
	{
		int client_index = check_client_is_live(index, argument);
		if (client_index != -1)
		{
			std::string nick = this->clients[client_index]->getNickname();
			std::string user = this->clients[client_index]->getUsername();
			std::string ip = this->clients[client_index]->getIP();
			std::string response = "User Info:\nNickname: " + nick + "\nUsername: " + user + "\nIP: " + ip + "\n";
			sendError(this->_fds[index].fd, response);
			return;
		}
	}
	else if (command == "!SERVER")
	{
		std::string Server_Name = "🟧 Orange Pixel 🟧";
		std::string clients_count = (g_num_fds - 1); // excluding server fd
		std::string channels_count = "N/A"; // Placeholder, implement channel tracking to get actual count
		std::string port_number = (this->port);
		std::string response = "Server Info:\nServer Name: " + Server_Name + "\nConnected Clients: " + clients_count + "\nActive Channels: " + channels_count + "\nPort Number: " + port_number + "\n";
		sendError(this->_fds[index].fd, response + "\r\n");
	}
	else if (command == "!ABOUT")
	{
		std::string Description = "Friendly IRC bot for ft_irc";
		std::string Version = "v1.0.0";
		std::string Language = "C++98";
		std::string Features = " !help !wanted !race !coin !roll";
		std::string Developers = "Developed by nbougrin and yabounna and oelbied";
		std::string response = "Bot Info:\nDescription: " + Description + "\nVersion: " + Version + "\nLanguage: " + Language + "\nFeatures: " + Features + "\nDevelopers: " + Developers + "\n";
		sendError(this->_fds[index].fd, response + "\r\n");
		

	}
	else if (command == "!BATTLE")
	{
		int client_index = check_client_is_live(index, argument);
		int outcome = rand() % 3; // 0 = win, 1 = draw, 2 = lose
		std::string result;

		if (argument.empty())
		{
			if (outcome == 0)
				result = "You won against yourself 🏆 (crazy skills)";
			else if (outcome == 1)
				result = "You fought your own brain and it rage quit 😭";
			else
				result = "You challenged yourself... and still lost 💀 -999 aura\n";
			sendError(this->_fds[index].fd, "Battle Result: " + result + "\n");
		}
		if (client_index == -1)
		{
			sendError(this->_fds[index].fd, "Battle: user '" + argument + "' not found ❌ try other user\r\n");
			return;
		}
		if (client_index != -1)
		{
			// Randomly determine battle outcome

			if (outcome == 0)
				result = "You win! 🏆\n";
			else if (outcome == 1)
				result = "draw 😐";
			else
				result = "you got destroyed 💀 you got +999 aura\n";

			sendError(this->_fds[index].fd, "Battle Result against " + argument + ": " + result + "\r\n");
			return;
		}
	}
	else if (command == "!GTA")
	{
		// GTA command implementation
		int client_index = check_client_is_live(index, argument);
		int outcome = rand() % 3; // 0 = win, 1 = draw, 2 = lose
		std::string result;

		if (argument.empty())
		{
			if (outcome == 0)
				result = "⭐⭐ lhnach 🚨 3la9\n";
			else if (outcome == 1)
				result = "⭐⭐⭐ hadchi khatir thala ajmil RUN 🏃‍♂️\n";
			else
				result = "⭐⭐⭐⭐⭐ L3AAAZWA 9ADIYA FIHA SWAT 🚁💥 \n";
			sendError(this->_fds[index].fd, "Battle Result: " + result + "\n");
		}

	}
	else
	{
		sendError(this->_fds[index].fd, "ERROR: Unknown bot command \"" + command + "\". Type !HELP for a list of commands.\r\n");
	}

}
