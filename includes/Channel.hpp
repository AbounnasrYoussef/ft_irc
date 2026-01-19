#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include <string>

class Client;

class Channel {
    private:
        std::string _name;
        std::vector<Client*> _members;
        std::vector<Client*> _operators;
        std::string _topic;
        std::string _key;
        
    public:
        Channel();
        Channel(const std::string& name);
        Channel(const Channel& other);
        
        void add_member(Client* client);
        void remove_member(Client* client);
        bool has_member(Client* client);
        std::vector<Client*> get_members();
        
        void add_operator(Client* client);
        void remove_operator(Client* client);
        bool is_operator(Client* client);
        
        std::string get_name();
        std::string get_topic();
        void set_topic(std::string topic);
        
        bool is_empty();
        
        Channel& operator=(const Channel& other);
        ~Channel();
};

#endif