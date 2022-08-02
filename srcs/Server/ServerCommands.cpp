#include "Server/Server.hpp"
#include "libft.h"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
#include "Log.hpp"

#include <map>
#include <iostream>

using std::string;

namespace irc {

/**
 * Command: NICK
 * Parameters: <nickname>
 */
void Server::NICK(Command &cmd, int fd) {

    int size = cmd.args.size();
    
    /* case too many params */
    if (size > 2) {
        return ;
    }
    /* case no nickname */
    if (size < 2) {
        string reply(ERR_NONICKNAMEGIVEN "*" STR_NONICKNAMEGIVEN);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    string real_nick = cmd.args[1];
    if (real_nick[0] == ':') {
        real_nick = real_nick.substr(1); // ignore : start.
    }
    string nick = real_nick; // nick is always upper case to ease lookup
    tools::ToUpperCase(nick);
    /* case forbidden characters are found / incorrect length */
    if (nickFormatOk(real_nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME+real_nick+STR_ERRONEUSNICKNAME);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    /* case nickname is equal to some other in the server
     * (ignoring upper/lower case) */
    if (nickExists(nick)) {
        string reply(ERR_NICKNAMEINUSE+real_nick+STR_NICKNAMEINUSE);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    User& user = getUserFromFd(fd);
    /* case nickname change */
    if (user.isResgistered()) {
        LOG(DEBUG) << "user with nick" << user.real_nick << "changing nick to " << real_nick;
        updateUserNick(fd, nick, real_nick);
        // THIS SHOULD NOTIFY CHANNELS OF THE NICKNAME CHANGE CYA
        return ;
    }
    /* case the nickname is the first recieved from this user */
    user.nick = nick;
    user.real_nick = real_nick;
    addNickFdPair(nick, fd);
    /* case nickname is recieved after valid USER command */
    if (!user.name.empty() && !user.full_name.empty()) {
        user.setPrefixFromHost(hostname);
        user.registered = true;
        user.server_mode = "o";
        return sendWelcome(user.name, user.prefix, fd);
    }
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void Server::USER(Command &cmd, int fd) {

    int size = cmd.args.size();
    User& user = getUserFromFd(fd);

    /* case many params (from irc-hispano) */
    if (size > 5) {
        return ;
    }
    /* case arguments unsufficient */
    if (size < 5) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    /* case user already sent a valid USER comand */
    if (user.isResgistered()) {
        return sendAlreadyRegistered(user.real_nick, fd);
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
    user.server_mode = "o";
    return sendWelcome(user.name, user.prefix, fd);
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

void Server::PING(Command &cmd, int fd) {
    
    int size = cmd.args.size();
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    User& user = getUserFromFd(fd);
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    string pong_reply("PONG " + cmd.args[1]);
    DataToUser(fd, pong_reply, NO_NUMERIC_REPLY);
}

/*
 * Command: PONG
 * Parameters: [<server>] <token>
 */
void Server::PONG(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);

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
 * 3. If channel doesn't exist, create new channel and new User with operator channel_mode and add it to the channel_map
 * 4. If channel exists, check channel_mode
 * 4.1. If channel is in channel_mode 'i', check if user is invited to join it. If not, error message is returned
 * 4.2. If channel is in channel_mode 'K', check if user is providing the correct pw. If not, error message is returned
 * 5. Add user to channel
 * 6. Send replies
 */
void Server::JOIN(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    //LOG(DEBUG) << user.nick << " password is " << user.connection_pass;
    int size = cmd.args.size();
    //LOG(DEBUG) << "join: started";
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    string ch_name = cmd.args[1];
    if (!tools::starts_with_mask(ch_name)) {
        return sendBadChannelMask(cmd.Name(), fd);
    }
    /* case channel does not exist */
    if (!channel_map.count(cmd.args[1])) {
        //LOG(DEBUG) << "join: channel doesn't exist";
        //LOG(DEBUG) << "join: creating channel with name " << ch_name;
        Channel channel(ch_name, user);
        //LOG(DEBUG) << user.name << " has channel_mode " << user.channel_mode << " after joining channel";
        channel.setUserMode(user, "o");
        if (size >= 3) {
            channel.key = cmd.args[2];
        }
        addNewChannel(channel);
        LOG(DEBUG) << "join: users size: " << channel.users.size();
        LOG(DEBUG) << user.name << " has channel_mode " << user.channel_mode.find(channel.name)->second << " after joining channel";
    /* case channel exists already */
    } else {
        LOG(DEBUG) << "join: channel exists";
        Channel &channel = getChannelFromName(ch_name);
        if (channel.userIsInChannel(fd)) {
            string reply(ERR_USERONCHANNEL" "+user.nick+" "+channel.name+" "STR_USERONCHANNEL);
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (channel.inviteModeOn()) {
            LOG(DEBUG) << "channel mode :" << channel.mode;
            if (!channel.isInvited(user)) {
                string reply(ERR_INVITEONLYCHAN+cmd.Name()+STR_INVITEONLYCHAN);
                return DataToUser(fd, reply, NUMERIC_REPLY);
            }
        } else if (channel.keyModeOn()) {
            if (size == 2
                || channel.key.compare(cmd.args[2]))
            {
                string reply(ERR_BADCHANNELKEY+cmd.Name()+STR_BADCHANNELKEY);
                return DataToUser(fd, reply, NUMERIC_REPLY);
            }
        }
        LOG(DEBUG) << "join: adding ch_user to existing channel";
        channel.addUser(user, channel.name, "");
        LOG(DEBUG) << user.name << " has channel_mode " << user.channel_mode.find(channel.name)->second << " after joining channel " << channel.name;
        LOG(DEBUG) << "join: users size: " << channel.users.size();
        if (channel.topicModeOn()) {
            string reply(RPL_TOPIC+user.nick+" "+channel.name+" :"+channel.topic);
            DataToUser(fd, reply, NUMERIC_REPLY);
        }
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
void Server::PART(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();
    
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
	std::cout << "Is user in channel " << !channel.userIsInChannel(fd);
    if (!channel.userIsInChannel(fd)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    LOG(DEBUG) << "part: deleting ch_user " << user.name << " in channel " << channel.name;
    LOG(DEBUG) << user.name << " has channel_mode " << user.channel_mode.find(channel.name)->second << " before being erased";
    channel.deleteUser(user);
    user.deleteChannelMask(channel.name);
    LOG(DEBUG) << "part: users size: " << channel.users.size();
    if (channel.users.size() == 0) {
        LOG(DEBUG) << "part: deleting empty channel";
        removeChannel(channel);
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
void Server::TOPIC(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(fd)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!tools::charIsInString(channel.mode, 't')) {
        return sendNoChannelModes(cmd.Name(), fd);
    }
    if (size == 2) {
        if (channel.topic.empty()) {
            string reply(RPL_NOTOPIC+cmd.Name()+STR_NOTOPIC);
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        string reply(RPL_TOPIC+user.nick+" "+channel.name+" :"+channel.topic);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (size == 3) {
        if (!channel.isUserOperator(user)) {
            return sendChannelOperatorNeeded(cmd.Name(), fd);
        }
        if (cmd.args[2].empty()) {
            channel.topic = "";
            return ;
        }
        channel.topic = cmd.args[2].substr(1);
        string reply(RPL_TOPIC+user.nick+" "+channel.name+" :"+channel.topic);
        return DataToUser(fd, reply, NUMERIC_REPLY);
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
void Server::KICK(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(cmd.Name(), fd);
    }
    if (!channel_map.count(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(fd)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd);
    }
    string nick = cmd.args[2];
    if (!channel.userIsInChannel(nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    User& user_to_kick = channel.getUserFromNick(nick);
    channel.banUser(user_to_kick); // Ban this user??
    channel.deleteUser(user_to_kick);

    if (size == 3) {
        // TODO: mandar mensajes por defecto y por parametro a los Users
    } else {

    }
}

/**
 * Command: INVITE
 * Parameters: <nickname> <channel>
 * 1. Check correct parameter size & if user and channel exist
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel, if it exists
 * 3. Reuse the PART method passing the comment as the part message
 */
void Server::INVITE(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    if (!channel_map.count(cmd.args[2])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[2])->second;
    if (!tools::charIsInString(channel.mode, 'i')) {
        return sendNoChannelModes(cmd.Name(), fd);
    }
    if (!channel.userIsInChannel(fd)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd);
    }
    if (!nick_fd_map.count(tools::toUpper(cmd.args[1]))) {
        string reply = (ERR_NOSUCHNICK + cmd.Name() + STR_NOSUCHNICK);
        LOG(DEBUG) << reply;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (channel.userIsInChannel(cmd.args[1])) {
        return ; // el usuario ya esta en el canal. no hace falta inv
    }
    string nick = cmd.args[1];
    tools::ToUpperCase(nick);
    User invited_user = getUserFromNick(nick);
    channel.addToWhitelist(invited_user);
    LOG(DEBUG) << "User is invited " << channel.isInvited(user);
    // TODO : else, añadir el usuario a la whitelist (cuando haya que hacerlo, usuario exista etc.)
    /*nick_fd_map.count(cmd.args[1]);
    LOG(DEBUG) << "invite: channel User " << user_to_inv.nick << " has been invited";
    channel.addToWhitelist(user_to_inv);
    LOG(DEBUG) << "invite: WhiteList size " << channel.white_list.size();
    if (size == 3) {
        // TODO: mandar mensajes por defecto y por parametro a los Users
    } else {

    }*/

}

/**
 *
 */

/**
 * Command: MODE
 * Parameters: <channel>/<user> <channel/userMode> [<modeParams>]
 * TODO
 */
void Server::MODE(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    // CHANGE CHANNEL MODE. COMPLEX PATH
    if (tools::starts_with_mask(cmd.args[1])) {
        if (size < 4) {
            return sendNeedMoreParams(cmd.Name(), fd);
        }
        if (!channel_map.count(cmd.args[1])) {
            return sendNoSuchChannel(cmd.Name(), fd);
        }
        Channel &channel = channel_map.find(cmd.args[1])->second;
        if (!channel.userIsInChannel(fd)) {
            return sendNotOnChannel(cmd.Name(), fd);
        }
        LOG(DEBUG) << user.channel_mode.find(channel.name)->second;
        LOG(DEBUG) << user.name << " has channel_mode " << user.channel_mode.find(channel.name)->second << " after joining channel " << channel.name;
        if (!channel.isUserOperator(user)) {
            return sendChannelOperatorNeeded(cmd.Name(), fd);
        }
        /* CYA : y si la string es +-i-----++tm---k  que se hace? esta condicion
         * solo cubre que tenga o no un +, no que tenga un + y una letra
         * siguiente. */
        if (tools::charIsInString(cmd.args[2], '+')) {
            if (tools::charIsInString(cmd.args[2], 'i')) {
                channel.mode.append("i");
            }
            if (tools::charIsInString(cmd.args[2], 't')) {
                channel.mode.append("t");
            }
            if (tools::charIsInString(cmd.args[2], 'm')) {
                channel.mode.append("m");
            }
            if (tools::charIsInString(cmd.args[2], 'k')) {
                if (size < 4) {
                    return sendNeedMoreParams(cmd.Name(), fd);
                }
                channel.mode.append("k");
                channel.key = cmd.args[3];
            }
        }
        if (tools::charIsInString(cmd.args[2], '-')) {
            if (tools::charIsInString(cmd.args[2], 'i')) {
                channel.mode.erase(channel.mode.find("i"));
            }
            if (tools::charIsInString(cmd.args[2], 't')) {
                channel.mode.erase(channel.mode.find("t"));
            }
            if (tools::charIsInString(cmd.args[2], 'm')) {
                channel.mode.erase(channel.mode.find("m"));
            }
            if (tools::charIsInString(cmd.args[2], 'k')) {
                if (size < 4) {
                    return sendNeedMoreParams(cmd.Name(), fd);
                }
                if (cmd.args[3].compare(channel.key)) {
                    string reply = (ERR_BADCHANNELKEY + cmd.Name() + STR_BADCHANNELKEY);

                }
                channel.mode.erase(channel.mode.find("k"));
            }
        }
    // CHANGE USER MODE
    } else {
        string mode = cmd.args[2];
        if (tools::hasUnknownFlag(mode)) {
            string reply = (ERR_UMODEUNKNOWNFLAG""STR_UMODEUNKNOWNFLAG);
            LOG(DEBUG) << reply;
            return DataToUser(fd, reply, NUMERIC_REPLY);

        }
        if (tools::charIsInString(mode, 'o')
            || tools::charIsInString(mode, 'O')) {
            if (tools::isEqual(cmd.args[1], user.nick)) {
                string reply = (ERR_USERSDONTMATCH + cmd.Name() + STR_USERSDONTMATCH);
                LOG(DEBUG) << reply;
                return DataToUser(fd, reply, NUMERIC_REPLY);
            }
        }
        // Check if user is trying to give itself an operator mode
        // Check if serverUser is operator
        /*if (user.server_mode.find("o") == string::npos
            || user.server_mode.find("O") == string::npos) {
            string reply = (ERR_CHANOPRIVSNEEDED + cmd.Name() + STR_CHANOPRIVSNEEDED);
            LOG(DEBUG) << reply;
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }*/
        // Check if serverUser is sending a different nickname
        if (!tools::isEqual(cmd.args[1], user.nick)) {
            string reply = (ERR_USERSDONTMATCH + cmd.Name() + STR_USERSDONTMATCH);
            LOG(DEBUG) << reply;
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (tools::charIsInString(mode, '+')
            && tools::charIsInString(mode, 'a')) {
            user.server_mode.append("a");
        }
        if (tools::charIsInString(mode, '-')
            && tools::charIsInString(mode, 'a')) {
            user.server_mode.erase(mode.find("a"));
        }
    }
}

/**
 * Command: PASS
 * Parameters: <password>
 * This command must be sent before user registration to set a connection password
 */
void Server::PASS(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    
    if (user.registered) {
        return sendAlreadyRegistered(user.real_nick, fd);
    }
    int size = cmd.args.size();
    if (size < 2) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    user.connection_pass = cmd.args[1];
}

/**
 * Command: QUIT
 * Parameters: [<quitMessage>]
 * The server aknowledges this by sending an error client to the server
 */
void Server::QUIT(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    string reply = "User " + user.nick + " has left the server.";
    reply = size == 2 ? user.nick.append(": ").append(cmd.args[1]) : reply;
    DataToUser(fd, reply, NO_NUMERIC_REPLY);
    // TODO : find the most effective way to erase the fd & mantain user data for possible reconnection
}

/**
 * Command: AWAY
 * Parameters: [<awayMessage>]
 * With parameter, sets afk_msg. Without, unsets it.
 * Always return if user is away or not
 */
void Server::AWAY(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    
    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size == 2) {
        user.afk_msg = cmd.args[3];
        LOG(DEBUG) << user.afk_msg;
    } else {
        user.afk_msg = "";
        LOG(DEBUG) << user.afk_msg;
    }
    if (tools::charIsInString(user.server_mode, 'a')) {
        string reply = (RPL_NOWAWAY STR_NOWAWAY);
        LOG(DEBUG) << user.server_mode;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    string reply = (RPL_UNAWAY STR_UNAWAY);
    LOG(DEBUG) << user.server_mode;
    return DataToUser(fd, reply, NUMERIC_REPLY);
}

} // namespace irc
