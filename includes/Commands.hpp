#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>


struct ParsedMessage {
    std::string target;      // destinataire
    std::string message;     // le message
    bool valid;              // succès du parsing
    int error_code;          // code d'erreur si !valid
};

ParsedMessage parse_arguments(const std::string &argument);
void trim(std::string& s);
















#endif