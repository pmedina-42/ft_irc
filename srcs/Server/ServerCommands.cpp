#include "Server/Server.hpp"
#include "libft.h"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
#include "ChannelUser.hpp"
#include "Log.hpp"

#include <map>
#include <iostream>

namespace irc {

/* See 
 * https://forums.mirc.com/ubbthreads.php/topics/186181/nickname-valid-characters */
static bool nickFormatOk(string &nickname) {

    if (nickname.empty() || nickname.length() > NAME_MAX_SIZE) { // not sure this can happen.
        return false;
    }
    for (string::iterator it = nickname.begin(); it != nickname.end(); it++) {
        if (ft_isalnum(*it) == 0
            && *it != '`' && *it != '|' && *it != '^' && *it != '_'
            && *it != '-' && *it != '{' && *it != '}' && *it != '['
            && *it != ']' && *it != '\\')
        {
            return false;
        }
    }
    return true;
}

bool nickExists(string &nickname, FdUserMap& map) {
    for (FdUserMap::iterator it = map.begin();
                        it != map.end(); it++)
    {
        if (tools::isEqual(nickname, it->second.nick)) {
            return true;
        }
    }
    return false;
}

void Server::sendPingToUser(int fd_idx) {

    User &user = getUserFromFdIndex(fd_idx);
    /* send ping message */
    string random = ":" + tools::rngString(10);
    string ping_msg("PING " + random);
    DataToUser(fd_idx, ping_msg, NO_NUMERIC_REPLY);

    user.updatePingStatus(random);
}

/**
 * Command: NICK
 * Parameters: <nickname>
 */
void Server::NICK(Command &cmd, int fd_idx) {

    int fd = getFdFromIndex(fd_idx);

    int size = cmd.args.size();
    /* case too many params */
    if (size > 2) {
        return ;
    }
    /* case no nickname */
    if (size < 2) {
        string reply(ERR_NONICKNAMEGIVEN "*" STR_NONICKNAMEGIVEN);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    string nick = cmd.args[1];
    if (nick[0] == ':') {
        nick = nick.substr(1); // ignore : start.
    }
    /* case forbidden characters are found / incorrect length */
    if (nickFormatOk(nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME+nick+STR_ERRONEUSNICKNAME);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    /* case nickname is equal to some other in the server
     * (ignoring upper/lower case) */
    if (nickExists(nick, fd_user_map)) {
        string reply(ERR_NICKNAMEINUSE+nick+STR_NICKNAMEINUSE);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    User& user = getUserFromFd(fd);
    /* case nickname change */
    if (user.registered == true) {
        /* since we are changing the key, erase + insert is needed */
        nick_fd_map.erase(user.nick);
        nick_fd_map.insert(std::make_pair(nick, fd));
        user.nick = nick;
        // THIS SHOULD NOTIFY CHANNELS OF THE NICKNAME CHANGE CYA
        return ;
    }
    /* case the nickname is the first recieved from this user */
    user.nick = nick;
    nick_fd_map.insert(std::make_pair(nick, fd));
    /* case nickname is recieved after valid USER command */
    if (!user.name.empty() && !user.full_name.empty()) {
        user.setPrefixFromHost(hostname);
        user.registered = true;
        return sendWelcome(user.name, user.prefix, fd_idx);
    }
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void Server::USER(Command &cmd, int fd_idx) {

    int size = cmd.args.size();
    /* case many params (from irc-hispano) */
    if (size > 5) {
        return ;
    }
    /* case arguments unsufficient */
    if (size < 5) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }

    User& user = getUserFromFdIndex(fd_idx);
    /* case user already sent a valid USER comand */
    if (user.registered == true) {
        string reply(ERR_ALREADYREGISTERED "" STR_ALREADYREGISTERED);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    /* wether nick exists or not, name and full name should be saved */
    user.name = cmd.args[1];
    string full_name = cmd.args[size - 1];
    if (full_name[0] == ':') {
        full_name = full_name.substr(1);
    }
    /* rare case USER cmd is recieved before nick (irc hispano allows this) */
    if (user.nick.empty()) {
        return ;
    }
    /* generic case (USER comand after NICK for registration) */
    user.setPrefixFromHost(hostname);
    user.registered = true;
    return sendWelcome(user.name, user.prefix, fd_idx);
}

/*
 * Command: PING
 * Parameters: <token> 
 * 
 * testeo con irc hispano :
 * PING :lol lasd as qwe q r  31412413r f13!"32º
 * :stirling.chathispano.com PONG stirling.chathispano.com :lol lasd as qwe q r  31412413r f13!"32º
 * PING :: : : : : :
 * :stirling.chathispano.com PONG stirling.chathispano.com :: : : : : :
 */

void Server::PING(Command &cmd, int fd_idx) {
    
    int size = cmd.args.size();
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }

    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    string pong_reply("PONG " + cmd.args[1]);
    DataToUser(fd_idx, pong_reply, NO_NUMERIC_REPLY);
}

/*
 * Command: PONG
 * Parameters: [<server>] <token>
 */
void Server::PONG(Command &cmd, int fd_idx) {

    User& user = getUserFromFdIndex(fd_idx);

    int size = cmd.args.size();
    /* PONG has no replies */
    if (size < 2
        /* case pong is recieved wihtout previous PING */
        || user.ping_str.empty())
    {
        return ;
    }
    if (user.ping_str.compare(cmd.args[1]) == 0) {
        user.resetPingStatus();
    } else {
        LOG(DEBUG) << "PING from user " << user << " incorrect, sent "
                   << user.ping_str << " recieved " << cmd.args[1];
    }
    return ;
}

/**
 * Command: JOIN
 * Parameters: <channel> [<key>]
 * 1. If command JOIN has no arguments, error message is returned
 * 2. If channel mask is missing, bad mask error is returned
 * 3. If channel doesn't exist, create new channel and new channelUser with operator mode and add it to the channel_map
 * 4. If channel exists, check mode
 * 4.1. If channel is in mode 'i', check if user is invited to join it. If not, error message is returned
 * 4.2. If channel is in mode 'K', check if user is providing the correct pw. If not, error message is returned
 * 5. Add user to channel
 * 6. Send replies
 */
void Server::JOIN(Command &cmd, int fd_idx) {
    int size = cmd.args.size();
    LOG(DEBUG) << "join: started";
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    string channel_name = cmd.args[1];
    ChannelUser channel_user(user);
    if (!tools::starts_with_mask(channel_name)) {
        return sendBadChannelMask(cmd.Name(), fd_idx);
    }
    /* case channel does not exist */
    if (!channel_map.count(cmd.args[1])) {
        LOG(DEBUG) << "join: channel doesn't exist";
        LOG(DEBUG) << "join: creating channel with name " << channel_name;
        Channel channel(channel_name, channel_user);
        LOG(DEBUG) << "channel user " << channel_user << " has mode " << channel_user.mode << " after joining channel";
        channel.setUserMode(channel_user, 'o');
        if (size >= 3) {
            channel.key = cmd.args[2];
        }
        channel_map.insert(std::make_pair(channel_name, channel));
        LOG(DEBUG) << "join: users size: " << channel.users.size();
    /* case channel exists already */
    } else {
        LOG(DEBUG) << "join: channel exists";
        
        Channel &channel = getChannelFromName(channel_name);
        if (channel.inviteModeOn()) {
            LOG(DEBUG) << channel.mode;
            if (!channel.isInvited(channel_user)) {
                string reply(ERR_INVITEONLYCHAN+cmd.Name()+STR_INVITEONLYCHAN);
                return DataToUser(fd_idx, reply, NUMERIC_REPLY);
            }
        } else if (channel.keyModeOn()) {
            if (size == 2
                || channel.key.compare(cmd.args[2]))
            {
                string reply(ERR_BADCHANNELKEY+cmd.Name()+STR_BADCHANNELKEY);
                return DataToUser(fd_idx, reply, NUMERIC_REPLY);
            }
        }
        LOG(DEBUG) << "join: adding user to existing channel";
        channel.addUser(channel_user);
        LOG(DEBUG) << channel_user.name << " has mode " << channel_user.mode << " after joining channel";
        LOG(DEBUG) << "join: users size: " << channel.users.size();
        // TODO: if user is already in channel?
        // TODO: send rpl messages
    }
}

/**
 * Command: PART
 * Parameters: <channel> [<partMessage>]
 * 1. If command PART has no arguments, error message is returned
 * 2. If channel mask is missing, bad mask error is returned
 * 3. In each channel, check if it exists and if user belongs to channel
 * 4. Delete user in channel
 * 5. If channel has no users left, destroy channel
 * 6. Send part or default message to channel
 */
void Server::PART(Command &cmd, int fd_idx) {
    int size = cmd.args.size();
    string reply;
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd_idx);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd_idx);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    ChannelUser &user = channel.userInChannel(channel, fd_idx);
    if (user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    LOG(DEBUG) << "part: deleting user " << user.name << " in channel " << channel.name;
    LOG(DEBUG) << user.name << " has mode " << user.mode << " before being erased";
    channel.deleteUser(user);
    LOG(DEBUG) << "part: users size: " << channel.users.size();
    if (channel.users.size() == 0) {
        LOG(DEBUG) << "part: deleting empty channel";
        channel_map.erase(channel_map.find(channel.name));
    }
    if (size == 3) {
        // TODO: mandar mensaje en los canales a los usuarios
    } else {

    }
}

/**
 * Command: TOPIC
 * Parameters: <channel> [<topic>]
 * 1. If command TOPIC has no arguments, error message is returned
 * 2. If channel doesn't start with mask, bad mask error is returned
 * 3. If command TOPIC has one argument, channel topic is returned if exists (channel, user in channel & topic)
 * 4. If command TOPIC has two arguments, channel topic is set (if user has the requested permissions)
 * 5. If command TOPIC has two arguments, and topic is and empty string, the channel topic is removed
 * 6. Return RLP_TOPIC to client
 *  If not, error message is returned
 */
void Server::TOPIC(Command &cmd, int fd_idx) {
    int size = cmd.args.size();
    string reply;
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd_idx);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd_idx);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    ChannelUser user = channel.userInChannel(channel, fd_idx);
    if (user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    if (channel.mode.find("t") != string::npos) {
        return sendNoCannelModes(cmd.Name(), fd_idx);
    }
    if (size == 2) {
        if (channel.topic.empty()) {
            reply = (RPL_NOTOPIC+cmd.Name()+STR_NOTOPIC);
            return DataToUser(fd_idx, reply, NUMERIC_REPLY);
        }
        reply = (RPL_TOPIC+cmd.Name()+STR_TOPIC);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    if (size == 3) {
        if (channel.isUserOperator(user)) {
            return sendChannelOperatorNeeded(cmd.Name(), fd_idx);
        }
        if (cmd.args[2].length() == 1) { // longitud 1 ? no sería .empty() ?
            channel.topic = "";
            return ;
        }
        channel.topic = cmd.args[2].substr(1);
        reply = (RPL_TOPIC+cmd.Name()+STR_TOPIC);
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    else {
        return ; // size > 3 
    }
}

/**
 * Command: KICK
 * Parameters: <channel> <user> [<comment>]
 * 1. Check correct parameter size & if user and channel exist
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel, if it exists
 * 3. Reuse the PART method passing the comment as the part message
 */
void Server::KICK(Command &cmd, int fd_idx) {
    int size = cmd.args.size();
    string reply;
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd_idx);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd_idx);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    ChannelUser user = channel.userInChannel(channel, fd_idx);
    if (user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd_idx);
    }
    ChannelUser userToKick = channel.findUserByName(cmd.args[2]);
    if (userToKick.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    channel.banUser(userToKick); // Ban this user??
    channel.deleteUser(userToKick);

    if (size == 3) {
        // TODO: mandar mensajes por defecto y por parametro a los channelUsers
    } else {

    }
}

/**
 * Command: KICK
 * Parameters: <channel> <user> [<comment>]
 * 1. Check correct parameter size & if user and channel exist
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel, if it exists
 * 3. Reuse the PART method passing the comment as the part message
 */
void Server::INVITE(Command &cmd, int fd_idx) {
    int size = cmd.args.size();
    string reply;
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    if (!channel_map.count(cmd.args[2])) {
        return sendNoSuchChannel(cmd.Name(), fd_idx);
    }
    Channel &channel = channel_map.find(cmd.args[2])->second;
    if (channel.mode.find("i") != string::npos) {
        return sendNoCannelModes(cmd.Name(), fd_idx);
    }
    ChannelUser &user = channel.userInChannel(channel, fd_idx);
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd_idx);
    }
    int fd_user_to_invite;
    if (!nick_fd_map.count(cmd.args[1])) {
        reply = (ERR_NOSUCHNICK + cmd.Name() + STR_NOSUCHNICK);
        LOG(DEBUG) << reply;
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    fd_user_to_invite = nick_fd_map.find(cmd.args[1])->second;

    // User& user_to_invite = fd_user_map.find(fd_user_to_invite)->second; !da error eb linux pq no se usa más adelante, supongo que luego sí se usará.
    ChannelUser user_to_invite(fd_user_to_invite); // Esto necesita un constructor copia !!! No está definido en channel user. Este Channel User solo tiene el fd escrito. 
    LOG(DEBUG) << "invite: channel user " << user_to_invite.nick << " has been invited";
    channel.addToWhitelist(user_to_invite);
    LOG(DEBUG) << "invite: whitelist size " << channel.whiteList.size();
    if (size == 3) {
        // TODO: mandar mensajes por defecto y por parametro a los channelUsers
    } else {

    }

}

} // namespace irc
