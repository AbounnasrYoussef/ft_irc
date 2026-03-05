#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include <string>
#include <set>

class Client;

class Channel
{
private:
    std::string            _name;
    std::vector<Client *>  _members;    // Liste unifiée des membres
    std::vector<Client *>  _operators;  // Liste des opérateurs
    std::set<Client *>     _invited;    // Liste des invités (mode +i)
    std::string            _topic;
    std::string            _key;

    bool _hasKey;
    bool _invite_only;      // Mode +i
    bool _topic_protected;  // Mode +t
    bool _moderated;        // Mode +m
    bool _no_external;      // Mode +n
    int  _user_limit;       // Mode +l (0 = pas de limite)

public:
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();

    // Gestion membres (interface unifiée - remplace _users et _members)
    void        addUser(Client *client);       // Ajoute + promeut op si premier
    bool        hasUser(Client *client) const; // Vérifie présence
    void        removeUser(Client *client);    // Retire membre + op si besoin
    std::string getuserList();                 // Liste nicks pour NAMES

    // Broadcast
    void broadcast(const std::string &message, Client *exclude);

    // Clé de canal (+k)
    void              setAkey(const std::string &key);
    const std::string &getkey() const;
    bool              hasAkeys() const;
    bool              checkAkey(const std::string &key) const;

    // Accès membres (pour KICK/MODE)
    void                  add_member(Client *client);
    void                  remove_member(Client *client);
    bool                  has_member(Client *client);
    std::vector<Client *> get_members();

    // Opérateurs
    void add_operator(Client *client);
    void remove_operator(Client *client);
    bool is_operator(Client *client);

    // Topic
    std::string get_topic();
    void        set_topic(std::string topic);

    // Invite
    void addUserInvite(Client *client);
    bool isInvited(Client *client) const;

    // Infos générales
    std::string get_name();
    bool        is_empty();

    // Getters modes
    bool is_invite_only() const;
    bool is_topic_protected() const;
    bool is_moderated() const;
    bool is_no_external() const;
    int  get_user_limit() const;

    // Setters modes
    void set_invite_only(bool value);
    void set_topic_protected(bool value);
    void set_moderated(bool value);
    void set_no_external(bool value);
    void set_user_limit(int limit);
};

#endif
