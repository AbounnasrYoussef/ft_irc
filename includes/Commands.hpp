#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>

struct ParsedMessage
{
    std::string target;
    std::string message;
    bool        valid;
    int         error_code;
};

struct ParsedKick
{
    std::string channel;
    std::string target_nick;
    std::string reason;
    bool        valid;
    int         error_code;
    std::string error_msg;
};

struct ModeChange
{
    char        mode;    // 'i', 't', 'o', 'k', 'l', etc.
    bool        adding;  // true = +, false = -
    std::string param;   // Pour +o nick, +k pass, +l limit
};

struct ParsedMode
{
    std::string              channel;
    std::vector<ModeChange>  changes;
    bool                     valid;
    int                      error_code;
    std::string              error_msg;
};

ParsedMessage parse_arguments(const std::string &argument);
ParsedKick    parse_kick_arguments(const std::string &argument);
ParsedMode    parse_mode_arguments(const std::string &argument);

void trim(std::string &s);

#endif
