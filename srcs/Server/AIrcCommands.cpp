#include "Server/AIrcCommands.hpp"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
#include "Log.hpp"
#include "Channel.hpp"

#include "libft.h"

#include <map>
#include <iostream>

namespace irc {

AIrcCommands::AIrcCommands(void)
:
    FdManager(),
    IrcDataBase()
{}

AIrcCommands::AIrcCommands(string &hostname, string &port)
:
    FdManager(hostname, port),
    IrcDataBase()
{}

AIrcCommands::AIrcCommands(const AIrcCommands& other)
:
    FdManager(other),
    IrcDataBase(other)
{}

AIrcCommands::~AIrcCommands()
{}


/**
 * Command: NICK
 * Parameters: <nickname>
 * TODO: si un usuario ya registrado se cambia el nick hay que resetearlo en todos lados/mapas
 */
void AIrcCommands::NICK(Command &cmd, int fd) {

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
        LOG(DEBUG) << "user with nick" << user.real_nick
                   << " changing nick to " << real_nick;
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
        user.addServerMask(OP);
        return sendWelcome(user.name, user.prefix, fd);
    }
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void AIrcCommands::USER(Command &cmd, int fd) {

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
    user.full_name = full_name;
    /* rare case USER cmd is recieved before nick (irc hispano allows this) */
    if (user.nick.empty()) {
        return ;
    }
    /* generic case (USER comand after NICK for registration) */
    user.setPrefixFromHost(hostname);
    user.registered = true;
    user.addServerMask(OP);
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

void AIrcCommands::PING(Command &cmd, int fd) {
    
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
void AIrcCommands::PONG(Command &cmd, int fd) {

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
void AIrcCommands::JOIN(Command &cmd, int fd) {

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
    if (!channelExists(cmd.args[1])) {
        Channel channel(ch_name, user);
        user.ch_name_mask_map.insert(std::pair<string, unsigned char>(channel.name, 0x80));
        if (size >= 3) {
            channel.key = cmd.args[2];
            channel.addMode(CH_PAS);
        }
        addNewChannel(channel);
        string namesReply = constructNamesReply(user.real_nick, channel);
        return (DataToUser(fd, namesReply, NO_NUMERIC_REPLY));
    /* case channel exists already */
    } else {
        Channel &channel = getChannelFromName(ch_name);
        if (channel.userIsInChannel(user.nick)) {
            string reply(ERR_USERONCHANNEL" "+user.nick+" "+channel.name+" "STR_USERONCHANNEL);
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (channel.inviteModeOn()) {
            if (!channel.isInvited(user.nick)) {
                string reply(ERR_INVITEONLYCHAN+cmd.Name()+STR_INVITEONLYCHAN);
                return DataToUser(fd, reply, NUMERIC_REPLY);
            }
        } // TODO : tiene sentido tener modo invitacion y contraseña a la vez?
        if (channel.keyModeOn()) {
            if (size == 2
                || channel.key.compare(cmd.args[2]))
            {
                string reply(ERR_BADCHANNELKEY+cmd.Name()+STR_BADCHANNELKEY);
                return DataToUser(fd, reply, NUMERIC_REPLY);
            }
        }
        channel.addUser(user);
        user.ch_name_mask_map.insert(std::pair<string, unsigned char>(channel.name, 0x00));
        if (channel.topicModeOn()) {
            string reply(RPL_TOPIC+user.nick+" "+channel.name+" :"+channel.topic);
            DataToUser(fd, reply, NUMERIC_REPLY);
        }
        string namesReply = constructNamesReply(user.real_nick, channel);
        return (DataToUser(fd, namesReply, NO_NUMERIC_REPLY));
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
void AIrcCommands::PART(Command &cmd, int fd) {

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
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    string next =  channel.getNextOpUser(user.nick);
    if (next.compare("")) {
        User &nextOp = getUserFromNick(next);
        nextOp.addChannelMask(channel.name, OP);
    }
    channel.deleteUser(user);
    user.ch_name_mask_map.erase(channel.name);
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
void AIrcCommands::TOPIC(Command &cmd, int fd) {

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
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!channel.topicModeOn()) {
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
void AIrcCommands::KICK(Command &cmd, int fd) {

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
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd);
    }
    string nick = cmd.args[2];
    tools::ToUpperCase(nick);
    if (!channel.userIsInChannel(nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    User& user_to_kick = getUserFromNick(nick);
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
void AIrcCommands::INVITE(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }
    if (!channelExists(cmd.args[2])) {
        return sendNoSuchChannel(cmd.Name(), fd);
    }
    Channel &channel = channel_map.find(cmd.args[2])->second;
    if (!channel.inviteModeOn()) {
        return sendNoChannelModes(cmd.Name(), fd);
    }
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(cmd.Name(), fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(cmd.Name(), fd);
    }
    string nick = cmd.args[1];
    tools::ToUpperCase(nick);
    if (!nick_fd_map.count(nick)) {
        string reply = (ERR_NOSUCHNICK + cmd.Name() + STR_NOSUCHNICK);
        LOG(DEBUG) << reply;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (channel.userIsInChannel(nick)) {
        string reply(ERR_USERONCHANNEL" "+user.nick+" "+nick+" "+channel.name+" "STR_USERONCHANNEL);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    channel.addToWhitelist(nick);
    // TODO: mandar mensajes por defecto y por parametro a los Users
}

/**
 *
 */

/**
 * Command: MODE
 * Parameters: <channel>/<user> <channel/userMode> [<modeParams>]
 * TODO
 */
void AIrcCommands::MODE(Command &cmd, int fd) {

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
        if (size < 3) {
            return sendNeedMoreParams(cmd.Name(), fd);
        }
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(cmd.Name(), fd);
        }
        Channel &channel = channel_map.find(cmd.args[1])->second;
        if (!channel.userIsInChannel(user.nick)) {
            return sendNotOnChannel(cmd.Name(), fd);
        }
        if (!channel.isUserOperator(user)) {
            return sendChannelOperatorNeeded(cmd.Name(), fd);
        }
        if (tools::anyRepeatedChar(cmd.args[2]) || tools::hasUnknownChannelFlag(cmd.args[2])) {
            string reply = (cmd.args[2] + ERR_UNKNOWNMODE + STR_UNKNOWNMODE);
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (tools::charIsInString(cmd.args[2], '+')) {
            if (tools::charIsInString(cmd.args[2], 'i')) {
                channel.addMode(CH_INV);
            }
            if (tools::charIsInString(cmd.args[2], 't')) {
                channel.addMode(CH_TOP);
            }
            if (tools::charIsInString(cmd.args[2], 'm')) {
                channel.addMode(CH_MOD);
            }
            if (tools::charIsInString(cmd.args[2], 'k')) {
                if (size < 4) {
                    return sendNeedMoreParams(cmd.Name(), fd);
                }
                channel.addMode(CH_PAS);
                channel.key = cmd.args[3];
            }
        }
        if (tools::charIsInString(cmd.args[2], '-')) {
            if (tools::charIsInString(cmd.args[2], 'i')) {
                channel.deleteMode(CH_INV);
            }
            if (tools::charIsInString(cmd.args[2], 't')) {
                channel.deleteMode(CH_TOP);
            }
            if (tools::charIsInString(cmd.args[2], 'm')) {
                channel.deleteMode(CH_MOD);
            }
            if (tools::charIsInString(cmd.args[2], 'k')) {
                if (size < 4) {
                    return sendNeedMoreParams(cmd.Name(), fd);
                }
                if (cmd.args[3].compare(channel.key)) {
                    string reply = (ERR_BADCHANNELKEY + cmd.Name() + STR_BADCHANNELKEY);
                    return DataToUser(fd, reply, NUMERIC_REPLY);
                }
                channel.deleteMode(CH_PAS);
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
            user.addServerMask(SV_AWAY);
        }
        if (tools::charIsInString(mode, '-')
            && tools::charIsInString(mode, 'a')) {
            user.deleteServerMask(SV_AWAY);
        }
    }
}

/**
 * Command: PASS
 * Parameters: <password>
 * This command must be sent before user registration to set a connection password
 */
void AIrcCommands::PASS(Command &cmd, int fd) {

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
void AIrcCommands::QUIT(Command &cmd, int fd) {

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
void AIrcCommands::AWAY(Command &cmd, int fd) {

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
    if (user.isAway()) {
        string reply = (RPL_NOWAWAY STR_NOWAWAY);
        LOG(DEBUG) << user.server_mode;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    string reply = (RPL_UNAWAY STR_UNAWAY);
    LOG(DEBUG) << user.server_mode;
    return DataToUser(fd, reply, NUMERIC_REPLY);
}

/**
 * Command: OPER
 * Parameters: <user> <pass>
 * Used by a client to gain operator privileges in server
 * TODO preguntar a CYA si se recibe ya la pass y coomo, que ahora prefiero seguir avanzando con otras cosas
 */
void AIrcCommands::OPER(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(cmd.Name(), fd);
    }

}

/**
 * Command: NAMES
 * Parameters: [<channel>]
 * Lists users visible to client or users in a channel
 */
void AIrcCommands::NAMES(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size == 2) {
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(cmd.Name(), fd);
        }
        Channel &channel = channel_map.find(cmd.args[1])->second;
        string reply = constructNamesReply(user.real_nick, channel);
        return (DataToUser(fd, reply, NO_NUMERIC_REPLY));
    }

}

/**
 * Command: LIST
 * Parameters: [<channel>]
 * Lists channel size & modes
 * */
void AIrcCommands::LIST(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        return sendNotRegistered(cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size == 2) {
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(cmd.Name(), fd);
        }
        Channel &channel = channel_map.find(cmd.args[1])->second;
        string reply = (RPL_LISTSTART + user.real_nick + STR_LISTSTART);
        DataToUser(fd, reply, NUMERIC_REPLY);
        reply = constructListReply(user.real_nick, channel);
        DataToUser(fd, reply, NUMERIC_REPLY);
        reply = (RPL_LISTEND + user.real_nick + STR_LISTEND);
        DataToUser(fd, reply, NUMERIC_REPLY);
    }

}

// PRIVATE METHODS

string AIrcCommands::constructNamesReply(string nick, Channel &channel) {
    char symbol = channel.secretModeOn() ? '@' : '=';
    unsigned long i = 0;
    string reply = (RPL_NAMREPLY + nick + " " + symbol + " " + channel.name + " " + ":");
    for (std::list<string>::iterator it = channel.users.begin(); it != channel.users.end(); it++) {
        User user = getUserFromNick(*it);
        if (user.isChannelOperator(channel.name)) {
            reply += "@";
        }
        reply += user.real_nick;
        reply += (++i < channel.users.size()) ? " " : "";
    }
    return reply;
}

string AIrcCommands::constructListReply(string nick, Channel &channel) {
    string mode = channel.keyModeOn() ? "k" : "";
    mode += channel.inviteModeOn() ? "i" : "";
    mode += channel.topicModeOn() ? "t" : "";
    mode = mode.length() != 0 ? ("+" + mode) : mode;
    string reply = (RPL_LIST + nick + " " + channel.name + " " + std::to_string(channel.users.size()) + " [" + mode + "]");
    return reply;
}

} // namespace irc
