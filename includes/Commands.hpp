#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>


struct ParsedMessage {
    std::string target;      // destinataire
    std::string message;     // le message
    bool valid;              // succès du parsing
    int error_code;          // code d'erreur si !valid
};

struct ParsedKick {
    std::string channel;
    std::string target_nick;
    std::string reason;        // Optionnel
    bool valid;
    int error_code;
    std::string error_msg;
};
struct ModeChange {
    char mode;        // 'i', 't', 'o', 'k', 'l', etc.
    bool adding;      // true = +, false = -
    std::string param; // Pour +o, +k, +l
};

struct ParsedMode {
    std::string channel;
    std::vector<ModeChange> changes;
    bool valid;
    int error_code;
    std::string error_msg;
};

ParsedMessage parse_arguments(const std::string &argument);
ParsedKick parse_kick_arguments(const std::string &argument);
void trim(std::string& s);
















#endif