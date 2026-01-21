#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>

#include <vector>
#include <string>

#include <set>
#include "Client.hpp"
#include <string>

class Client;

class Channel
{
private:
    std::string _name;
    std::vector<Client *> _members;
    std::vector<Client *> _operators;
    std::set<Client *> _users;
    std::string _topic;
    std::string _key;
    bool hasAkey;
    bool _invite_only;     // Mode +i
    bool _topic_protected; // Mode +t
    bool _moderated;       // Mode +m
    bool _no_external;     // Mode +n
    int _user_limit;       // Mode +l (0 = pas de limite)

public:
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    // add for join comend
    void addUser(Client *client);
    // void removeUser(Client *client);
    bool hasUser(Client *Client) const;
    std::string getuserList();
    // const std::string& getName() const;
    // const std::set<Client*>& getUsers() const;

    void broadcast(const std::string &message, Client *exclude);
    void setAkey(const std::string &key);
    const std::string &getkey() const;
    bool hasAkeys() const;
    bool checkAkey(const std::string &key) const;

    void add_member(Client *client);
    void remove_member(Client *client);
    bool has_member(Client *client);
    std::vector<Client *> get_members();

    void add_operator(Client *client);
    void remove_operator(Client *client);
    bool is_operator(Client *client);

    std::string get_name();
    std::string get_topic();
    void set_topic(std::string topic);

    bool is_empty();

    // mode
    bool is_invite_only() const;
    bool is_topic_protected() const;
    bool is_moderated() const;
    bool is_no_external() const;
    int get_user_limit() const;

    void set_invite_only(bool value);
    void set_topic_protected(bool value);
    void set_moderated(bool value);
    void set_no_external(bool value);
    void set_user_limit(int limit);

    Channel &operator=(const Channel &other);
    ~Channel();
};

#endif

