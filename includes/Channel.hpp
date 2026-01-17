#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <vector>

class Client;

class Channel {
	private:
		std::string _name;
		std::vector<Client*> _members;
		std::vector<Client*> _operators;
		std::string _topic;
		std::string _key; // 3la 3bol motdepasse
	public:
		Channel();
		Channel(const std::string& name);
		Channel(const Channel& other);

		void add_member(Client* client);
    	void remove_member(Client* client);
		bool has_member(Client* client);
		std::vector<Client*> get_members();
		
		// Gestion des opérateurs
		void add_operator(Client* client);
		void remove_operator(Client* client);
		bool is_operator(Client* client);
		
		// Getters/Setters
		std::string get_name();
		std::string get_topic();
		void set_topic(std::string topic);
		
		bool is_empty();

		Channel& operator=(const Channel& other);
		~Channel();
};
#endif