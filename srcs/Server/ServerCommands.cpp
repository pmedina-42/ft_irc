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
        user.server_mode = "+o";
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
        return sendAlreadyRegistered(fd_idx);
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
    user.server_mode = "+o"; // ???
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
 * 3. If channel doesn't exist, create new channel and new channelUser with operator channel_mode and add it to the channel_map
 * 4. If channel exists, check channel_mode
 * 4.1. If channel is in channel_mode 'i', check if user is invited to join it. If not, error message is returned
 * 4.2. If channel is in channel_mode 'K', check if user is providing the correct pw. If not, error message is returned
 * 5. Add user to channel
 * 6. Send replies
 */
void Server::JOIN(Command &cmd, int fd_idx) {
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    LOG(DEBUG) << user.nick << " password is " << user.connection_pass;
    int size = cmd.args.size();
    string reply;
    LOG(DEBUG) << "join: started";
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    string ch_name = cmd.args[1];
    ChannelUser ch_user(user);
    if (!tools::starts_with_mask(ch_name)) {
        return sendBadChannelMask(cmd.Name(), fd_idx);
    }
    if (!channel_map.count(cmd.args[1])) {
        LOG(DEBUG) << "join: channel doesn't exist";
        LOG(DEBUG) << "join: creating channel with name " << ch_name;
        Channel channel(ch_name, ch_user);
        LOG(DEBUG) << ch_user.user.name << " has channel_mode " << ch_user.channel_mode << " after joining channel";
        channel.setUserMode(ch_user, 'o');
        channel_map.insert(std::make_pair(ch_name, channel));
        if (size >= 3) {
            channel.key = cmd.args[2];
        }
        LOG(DEBUG) << "join: users size: " << channel.users.size();
    } else {
        LOG(DEBUG) << "join: channel exists";
        Channel &channel = channel_map.find(ch_name)->second;
        if (channel.inviteModeOn()) {
            LOG(DEBUG) << channel.mode;
            if (!channel.isInvited(ch_user)) {
                reply = (ERR_INVITEONLYCHAN+cmd.Name()+STR_INVITEONLYCHAN);
                LOG(DEBUG) << reply;
                return DataToUser(fd_idx, reply, NUMERIC_REPLY);
            }
        } else if (channel.keyModeOn()) {
            if (size == 2
                || channel.key.compare(cmd.args[2]))
            {
                reply = (ERR_BADCHANNELKEY+cmd.Name()+STR_BADCHANNELKEY);
                LOG(DEBUG) << reply;
                return DataToUser(fd_idx, reply, NUMERIC_REPLY);
            }
        }
        LOG(DEBUG) << "join: adding ch_user to existing channel";
        channel.addUser(ch_user);
        LOG(DEBUG) << ch_user.user.name << " has channel_mode " << ch_user.channel_mode << " after joining channel";
        LOG(DEBUG) << "join: users size: " << channel.users.size();
        // TODO: if ch_user is already in channel?
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
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    int size = cmd.args.size();
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
    ChannelUser &ch_user = channel.userInChannel(fd_idx);
    if (ch_user.user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    LOG(DEBUG) << "part: deleting ch_user " << ch_user.user.name << " in channel " << channel.name;
    LOG(DEBUG) << ch_user.user.name << " has channel_mode " << ch_user.channel_mode << " before being erased";
    channel.deleteUser(ch_user);
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
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
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
    ChannelUser ch_user = channel.userInChannel(fd_idx);
    if (ch_user.user.fd == 0) {
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
        if (channel.isUserOperator(ch_user)) {
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
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
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
    ChannelUser ch_user = channel.userInChannel(fd_idx);
    if (ch_user.user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    if (!channel.isUserOperator(ch_user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd_idx);
    }
    ChannelUser user_to_kick = channel.findUserByNick(cmd.args[2]);
    if (user_to_kick.user.fd == 0) {
        return sendNotOnChannel(cmd.Name(), fd_idx);
    }
    channel.banUser(user_to_kick); // Ban this user??
    channel.deleteUser(user_to_kick);

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
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
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
    ChannelUser &ch_user = channel.userInChannel(fd_idx);
    if (!channel.isUserOperator(ch_user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd_idx);
    }
    if (!nick_fd_map.count(cmd.args[1])) {
        reply = (ERR_NOSUCHNICK + cmd.Name() + STR_NOSUCHNICK);
        LOG(DEBUG) << reply;
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    int fd_user_to_invite = nick_fd_map.find(cmd.args[1])->second;
    ChannelUser user_to_inv(getUserFromFd(fd_user_to_invite));

    LOG(DEBUG) << "invite: channel channelUser " << user_to_inv.user.nick << " has been invited";
    channel.addToWhitelist(user_to_inv);
    LOG(DEBUG) << "invite: WhiteList size " << channel.white_list.size();
    if (size == 3) {
        // TODO: mandar mensajes por defecto y por parametro a los channelUsers
    } else {

    }

}

/**
 *
 */

/**
 * Command: MODE
 * Parameters: <channel>/<user> <channel/userMode> [<modeParams>]
 * TODO
 */
void Server::MODE(Command &cmd, int fd_idx) {
    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    // CHANGE CHANNEL MODE. COMPLEX PATH
    if (tools::starts_with_mask(cmd.args[1])) {

    // CHANGE USER MODE
    } else {
        // Check if user is trying to give itself an operator mode
        // Check if serverUser is operator
        if (user.server_mode.find("o") == string::npos
            || user.server_mode.find("O") == string::npos) {
            string reply = (ERR_CHANOPRIVSNEEDED + cmd.Name() + STR_CHANOPRIVSNEEDED);
            LOG(DEBUG) << reply;
            return DataToUser(fd_idx, reply, NUMERIC_REPLY);
        }
        // Check if serverUser is sending a different nickname
        if (cmd.args[1].compare(user.nick) != 0) {
            string reply = (ERR_USERSDONTMATCH + cmd.Name() + STR_USERSDONTMATCH);
            LOG(DEBUG) << reply;
            return DataToUser(fd_idx, reply, NUMERIC_REPLY);
        }

    }
}

/**
 * Command: PASS
 * Parameters: <password>
 * This command must be sent before user registration to set a connection password
 */
void Server::PASS(Command &cmd, int fd_idx) {
    User &user = getUserFromFdIndex(fd_idx);
    if (user.registered) {
        return sendAlreadyRegistered(fd_idx);
    }
    int size = cmd.args.size();
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd_idx);
    }
    user.connection_pass = cmd.args[1];
}

/**
 * Command: QUIT
 * Parameters: [<quitMessage>]
 * The server aknowledges this by sending an error client to the server
 */
void Server::QUIT(Command &cmd, int fd_idx) {
    User &user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    int size = cmd.args.size();
    string reply = "User " + user.nick + " has left the server.";
    reply = size == 2 ? user.nick.append(": ").append(cmd.args[1]) : reply;
    DataToUser(fd_idx, reply, NO_NUMERIC_REPLY);
    // TODO : find the most effective way to erase the fd & mantain user data for possible reconnection
}

/**
 * Command: AWAY
 * Parameters: [<awayMessage>]
 * With parameter, sets afk_msg. Without, unsets it.
 * Always return if user is away or not
 */
void Server::AWAY(Command &cmd, int fd_idx) {
    User &user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegistered(cmd.Name(), fd_idx);
    }
    int size = cmd.args.size();
    if (size == 2) {
        user.afk_msg = cmd.args[3];
        LOG(DEBUG) << user.afk_msg;
    } else {
        user.afk_msg = "";
        LOG(DEBUG) << user.afk_msg;
    }
    if (user.server_mode.find("+a") != string::npos) {
        string reply = (RPL_NOWAWAY STR_NOWAWAY);
        LOG(DEBUG) << user.server_mode;
        return DataToUser(fd_idx, reply, NUMERIC_REPLY);
    }
    string reply = (RPL_UNAWAY STR_UNAWAY);
    LOG(DEBUG) << user.server_mode;
    return DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

} // namespace irc
