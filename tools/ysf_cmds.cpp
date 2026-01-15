#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


void handel_privmsg(std::string target, std::string message)
{
    int i = 0;
    if(target[i] == '#' || target[i] == '&')
    {
        // donc c'est un channel
        // 5assni nssay fonction send to channel
    }
    else // c'est un utilisateur
    {
        // fonction send to user
    }
}

void trim(std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");

    if (start == std::string::npos)
        s = "";
    else
        s = s.substr(start, end - start + 1);
}



ParsedMessage parse_arguments(const std::string &argument)
{
    ParsedMessage result;
    result.valid = false;
    
    // Chercher le ':'
    size_t pos = argument.find(':');
    
    // Pas de ':' → ERR_NOTEXTTOSEND
    if (pos == std::string::npos)
    {
        result.error_code = 412;
        return result;
    }
    
    // Extraire destinataire
    result.target = argument.substr(0, pos);
    trim(result.target);
    
    // Destinataire vide → ERR_NORECIPIENT
    if (result.target.empty())
    {
        result.error_code = 411;
        return result;
    }
    
    // Extraire message (ne pas trim!)
    result.message = argument.substr(pos + 1);
    
    // les messages vides
    // if (result.message.empty())
    // {
    //     result.error_code = 412;
    //     return result;
    // }
    result.valid = true;
    return result;
    
}
