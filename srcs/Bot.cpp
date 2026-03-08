#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

bool check_bot_command(const std::string& command)
{
	return (command == "!HELP" || command == "!USER" || command == "!SERVER" || command == "!ABOUT" ||
			command == "!RULES" || command == "!BATTLE");
}

int Server::check_client_is_live(int index, std::string aragument)
{
	for (size_t i = 1; i < this->_fds.size(); i++)
	{
		if (this->clients[i]->getNickname() == aragument)
			return i;
	}
	sendError(this->_fds[index].fd, "ERROR : No such this nick \"" + aragument + "\"\r\n");
	return -1;
}
void Server::help_command(int index)
{
	std::string msg;

		msg += "=== ft_irc HELP ===\n";
		msg += "To register (mandatory):\n";
		msg += "  1) PASS <password>                                      -> set server password (must be first and password only digits)\n";
		msg += "  2) NICK <nickname>                                      -> choose your nickname (only letters)\n";
		msg += "  3) USER <username> <hostname> <servername> <realname>   -> set username + realname\n";
		msg += "Example:\n";
		msg += "  PASS 123\n";
		msg += "  NICK JohnDoe\n";
		msg += "  USER guest tolmoon tolsun :Ronnie\n";
		msg += "\n";
		msg += "Basic commands:\n";
		msg += "  JOIN #chan            -> join/create a channel\n";
		msg += "  PRIVMSG <t> :<msg>    -> send message to user or channel\n";
		msg += "  QUIT :<reason>        -> leave server\n";
		msg += "\n";
		msg += "Channel operator commands:\n";
		msg += "  KICK #chan <nick> :<reason>   -> kick a user\n";
		msg += "  INVITE <nick> #chan           -> invite user to channel\n";
		msg += "  TOPIC #chan :<topic>          -> set channel topic\n";
		msg += "  MODE  #chan <modes>           -> change channel modes\n";
		msg += "\n";
		msg += "Bot fun commands:\n";
		msg += "  !battle [nick]  -> random fight result\n";
		msg += "\n";

		sendError(this->_fds[index].fd, msg);
		return;
}
void Server::bot(std::string command, std::string argument, int index)
{
	if (this->clients[index]->isPassOk() == false)
	{
		sendError(this->_fds[index].fd, "ERROR: You must be registered to use bot commands. Use PASS, NICK, and USER to register.\r\n");
		return;
	}
	if (command == "!USER")
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
		std::stringstream num_clients;
		std::stringstream port;
		num_clients << (this->_fds.size() - 1);
		port << this->port;

		std::string Server_Name = "🟧 Orange Pixel 🟧";
		std::string clients_count = num_clients.str(); // excluding server fd
		std::string port_number = port.str();
		std::string response = "Server Info:\nServer Name: " + Server_Name + "\nConnected Clients: " + clients_count + "\n" + "\nPort Number: " + port_number + "\n";
		sendError(this->_fds[index].fd, response + "\r\n");
		return;
	}
	else if (command == "!ABOUT")
	{
		std::string botName     = "MarvinBot";
		std::string description = "Friendly ft_irc bonus bot (fun + mini-games)";
		std::string version     = "v1.0.0";
		std::string language    = "C++98";
		std::string features    = "!help !battle";
		std::string developers  = "nbougrin, yabounna, oelbied";

		std::string response;
		response += "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
		response += "┃ 🤖 " + botName + " - ABOUT\n";
		response += "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n";
		response += "┃ Description : " + description + "\n";
		response += "┃ Version     : " + version + "\n";
		response += "┃ Language    : " + language + "\n";
		response += "┃ Features    : " + features + "\n";
		response += "┃ Developers  : " + developers + "\n";
		response += "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";

		sendError(this->_fds[index].fd, response + "\r\n");
		return;
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
			return;
		}
		if (client_index == -1)
		{
			sendError(this->_fds[index].fd, "Battle: user '" + argument + "' not found ❌\r\n");
			return;
		}
		if (client_index != -1)
		{
			if (outcome == 0)
				result = "You win! 🏆\n";
			else if (outcome == 1)
				result = "Draw 😐";
			else
				result = "You got destroyed 💀 you got +999 aura\n";

			sendError(this->_fds[index].fd, "Battle Result against " + argument + ": " + result + "\r\n");
			return;
		}
	}
	else
		sendError(this->_fds[index].fd, "ERROR: Unknown bot command \"" + command + "\". Type !HELP for a list of commands.\r\n");

}
