#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Commands.hpp"

ParsedKick parse_kick_arguments(const std::string &argument)
{
    ParsedKick result;
    result.valid = false;
    result.reason = "";

    // Extract optional reason after ':'
    size_t colon_pos = argument.find(':');
    std::string params;

    if (colon_pos != std::string::npos)
    {
        result.reason = argument.substr(colon_pos + 1);
        params = argument.substr(0, colon_pos);
    }
    else
    {
        params = argument;
    }

    trim(params);

    // Split into channel and target nick
    size_t space_pos = params.find(' ');

    if (space_pos == std::string::npos)
    {
        result.error_code = 461;
        result.error_msg = "KICK :Not enough parameters";
        return result;
    }

    result.channel = params.substr(0, space_pos);
    result.target_nick = params.substr(space_pos + 1);

    trim(result.channel);
    trim(result.target_nick);

    if (result.channel.empty() || result.target_nick.empty())
    {
        result.error_code = 461;
        result.error_msg = "KICK :Not enough parameters";
        return result;
    }

    result.valid = true;
    return result;
}

static std::string format_kick(Client *kicker, const std::string &channel,
                                const std::string &target, const std::string &reason)
{
    std::string result = ":";
    result += kicker->getNickname();
    result += "!";
    result += kicker->getUsername();
    result += "@";
    result += kicker->getIP();
    result += " KICK ";
    result += channel;
    result += " ";
    result += target;

    if (!reason.empty())
    {
        result += " :";
        result += reason;
    }

    result += "\r\n";
    return result;
}

void Server::handle_kick(int kicker_index, const std::string &argument)
{
    Client *kicker = this->clients[kicker_index];

    // 1. Parse arguments
    ParsedKick parsed = parse_kick_arguments(argument);

    if (!parsed.valid)
    {
        std::string error = ":server 461 ";
        error += kicker->getNickname();
        error += " KICK :Not enough parameters\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 2. Check channel exists
    Channel *channel = this->get_channel(parsed.channel);
    if (!channel)
    {
        std::string error = ":server 403 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :No such channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 3. Kicker must be in the channel
    //    Use hasUser() — consistent with JOIN which uses addUser()/_users
    if (!channel->hasUser(kicker))
    {
        std::string error = ":server 442 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not on that channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 4. Kicker must be operator
    if (!channel->is_operator(kicker))
    {
        std::string error = ":server 482 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.channel;
        error += " :You're not channel operator\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 5. Find target and verify they are in the channel
    Client *target = this->get_client_by_nickname(parsed.target_nick);
    if (!target || !channel->hasUser(target))
    {
        std::string error = ":server 441 ";
        error += kicker->getNickname();
        error += " ";
        error += parsed.target_nick;
        error += " ";
        error += parsed.channel;
        error += " :They aren't on that channel\r\n";
        sendError(kicker->get_fd(), error);
        return;
    }

    // 6. Format and send KICK message to ALL members including the target
    //    broadcast(msg, NULL) sends to everyone in _users (no exclusion)
    std::string kick_msg = format_kick(kicker, parsed.channel,
                                       parsed.target_nick, parsed.reason);
    channel->broadcast(kick_msg, NULL);

    // 7. Remove target from _members and _operators (remove_member covers both)
    channel->remove_member(target);
    channel->remove_operator(target);

    // 8. Clean up empty channel
    //    getuserList() iterates _users — if empty the channel has no members
    if (channel->getuserList().empty())
        this->delete_channel(channel);
}
