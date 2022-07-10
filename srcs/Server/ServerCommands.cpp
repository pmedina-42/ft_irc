#include "Server.hpp"
#include "libft.h"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
#include "ChannelUser.hpp"

#include <map>
#include <iostream>

namespace irc {

/* See 
 * https://forums.mirc.com/ubbthreads.php/topics/186181/nickname-valid-characters */
static bool nickFormatOk(string &nickname) {

    if (nickname.empty() || nickname.length() > 9) { // not sure this can happen.
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
        if (tools::is_equal(nickname, it->second.nick)) {
            return true;
        }
    }
    return false;
}

void Server::sendNeedMoreParamsMsg(string& cmd_name, int fd_idx) {
    string reply(ERR_NEEDMOREPARAMS+cmd_name+STR_NEEDMOREPARAMS);
    DataToUser(fd_idx, reply);
}

void Server::sendNotRegisteredMsg(string &cmd_name, int fd_idx) {
    string reply(ERR_NOTREGISTERED+cmd_name+STR_NOTREGISTERED);
    DataToUser(fd_idx, reply);
}

void Server::sendWelcomeMsg(string& name, string &prefix, int fd_idx) {
    string welcome_msg(RPL_WELCOME+name+RPL_WELCOME_STR_1+prefix);
    DataToUser(fd_idx, welcome_msg);
}

/**
 * Command: NICK
 * Parameters: <nickname>
 */
void Server::NICK(Command &cmd, int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;

    int size = cmd.args.size();
    /* case too many params */
    if (size > 2) {
        return ;
    }
    /* case no nickname */
    if (size < 2) {
        string reply(ERR_NONICKNAMEGIVEN "*" STR_NONICKNAMEGIVEN);
        return DataToUser(fd_idx, reply);
    }
    string nick = cmd.args[1];
    /* case forbidden characters are found / incorrect length */
    if (nickFormatOk(nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME+nick+STR_ERRONEUSNICKNAME);
        return DataToUser(fd_idx, reply);
    }
    /* case nickname is equal to some other in the server
     * (ignoring upper/lower case) */
    if (nickExists(nick, fd_user_map)) {
        string reply(ERR_NICKNAMEINUSE+nick+STR_NICKNAMEINUSE);
        return DataToUser(fd_idx, reply);
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
        return sendWelcomeMsg(user.name, user.prefix, fd_idx);
    }
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void Server::USER(Command &cmd, int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;

    int size = cmd.args.size();
    /* case many params (from irc-hispano) */
    if (size > 5) {
        return ;
    }
    /* case arguments unsufficient */
    if (size < 5) {
        return sendNeedMoreParamsMsg(cmd.Name(), fd_idx);
    }
    User& user = getUserFromFd(fd);
    /* case user already sent a valid USER comand */
    if (user.registered == true) {
        string reply(ERR_ALREADYREGISTERED "" STR_ALREADYREGISTERED);
        return DataToUser(fd_idx, reply);
    }
    /* wether nick exists or not, name and full name should be saved */
    user.name = cmd.args[1];
    user.full_name = cmd.args[size - 1];
    /* rare case USER cmd is recieved before nick (irc hispano allows this) */
    if (user.nick.empty()) {
        return ;
    }
    /* generic case (USER comand after NICK for registration) */
    user.setPrefixFromHost(hostname);
    user.registered = true;
    return sendWelcomeMsg(user.name, user.prefix, fd_idx);
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
    int fd = fd_manager.fds[fd_idx].fd;
    User& user = getUserFromFd(fd);

    if (size < 2) {
        return sendNeedMoreParamsMsg(cmd.Name(), fd_idx);
    }
    if (!user.registered) {
        return sendNotRegisteredMsg(cmd.Name(), fd_idx);
    }
    string pong_reply(" PONG " + cmd.args[1]);
    DataToUser(fd_idx, pong_reply);
}

/*
 * Command: PONG
 * Parameters: [<server>] <token>
 */
void Server::PONG(Command &cmd, int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;

    int size = cmd.args.size();
    User& user = getUserFromFd(fd);
    /* PONG has no replies */
    if (size < 2
        /* case pong is recieved wihtout previous PING */
        || user.ping_str.empty())
    {
        return ;
    }
    if (user.ping_str.compare(cmd.args[1]) == 0) {
        // update user Keep Alive state, everything ok.
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
        string reply;
    if (size < 2) {
        reply = (ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
        DataToUser(fd_idx, reply);
        return ;
    }
    string channelName = cmd.args[1];
    ChannelUser user(fd_idx);
    if (tools::starts_with_mask(channelName)) {
        if (!channel_map.count(cmd.args[1])) {
            Channel channel(channelName, user);
            channel.setUserMode(user, 'o');
            channel_map.insert(std::make_pair(channelName, channel));
            if (!cmd.args[2].empty()) {
                channel.key = cmd.args[2];
            }
        } else {
            Channel channel = channel_map.find(channelName)->second;
            if (channel.inviteModeOn()) {
                if (!channel.isInvited(user)) {
                    reply = (ERR_INVITEONLYCHAN+cmd.Name()+STR_INVITEONLYCHAN);
                    DataToUser(fd_idx, reply);
                    return ;
                }
            } else if (channel.keyModeOn()) {
                if (cmd.args[2].empty() || channel.key.compare(cmd.args[2])) {
                    reply = (ERR_BADCHANNELKEY+cmd.Name()+STR_BADCHANNELKEY);
                    DataToUser(fd_idx, reply);
                    return ;
                }
            }
            channel.addUser(user);
            // TODO: send rpl messages
        }
    } else {
        reply = (ERR_BADCHANMASK+cmd.Name()+STR_BADCHANMASK);
        DataToUser(fd_idx, reply);
        return ;
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
            reply = (ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
            DataToUser(fd_idx, reply);
            return ;
        }
        if (tools::starts_with_mask(cmd.args[1])) {
            if (!channel_map.count(cmd.args[1])) {
                reply = (ERR_NOSUCHCHANNEL+cmd.Name()+STR_NOSUCHCHANNEL);
                DataToUser(fd_idx, reply);
                return ;
            }
            Channel channel = channel_map.find(cmd.args[1])->second;
            ChannelUser user = channel.userInChannel(channel, fd_idx);
            if (user == NULL) {
                reply = (ERR_NOTONCHANNEL+cmd.Name()+STR_NOTONCHANNEL);
                DataToUser(fd_idx, reply);
                return ;
            }
            channel.deleteUser(user);
            if (channel.users.size() == 0) {
                channel_map.erase(channel_map.find(channel.name));
            }
        } else {
            reply = (ERR_BADCHANMASK+cmd.Name()+STR_BADCHANMASK);
            DataToUser(fd_idx, reply);
            return ;
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
        string reply = (ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
        DataToUser(fd_idx, reply);
        return ;
    }
    if (size >= 2) {
        if (tools::starts_with_mask(cmd.args[1])) {
            if (!channel_map.count(cmd.args[1])) {
                reply = (ERR_NOSUCHCHANNEL+cmd.Name()+STR_NOSUCHCHANNEL);
                DataToUser(fd_idx, reply);
                return ;
            }
            Channel channel = channel_map.find(cmd.args[1])->second;
            ChannelUser user = channel.userInChannel(channel, fd_idx);
            if (user == NULL) {
                reply = (ERR_NOTONCHANNEL+cmd.Name()+STR_NOTONCHANNEL);
                DataToUser(fd_idx, reply);
                return ;
            }
            if (channel.mode.find("t")) {
                reply = (ERR_NOCHANMODES+cmd.Name()+STR_NOCHANMODES);
                DataToUser(fd_idx, reply);
                return ;
            }
            if (size == 2) {
                if (channel.topic.empty()) {
                    reply = (RPL_NOTOPIC+cmd.Name()+STR_NOTOPIC);
                    DataToUser(fd_idx, reply);
                    return ;
                }
                reply = (RPL_TOPIC+cmd.Name()+STR_TOPIC);
                DataToUser(fd_idx, reply);
                return ;
            }
            if (size == 3) {
                if (channel.isUserOperator(user)) {
                    reply = (ERR_CHANOPRIVSNEEDED+cmd.Name()+STR_CHANOPRIVSNEEDED);
                    DataToUser(fd_idx, reply);
                    return ;
                }
                if (cmd.args[2].length() == 1) {
                    channel.topic = "";
                    return ;
                }
                channel.topic = cmd.args[2].substr(1);
                reply = (RPL_TOPIC+cmd.Name()+STR_TOPIC);
                DataToUser(fd_idx, reply);
                return ;
            }
        } else {
            reply = (ERR_BADCHANMASK+cmd.Name()+STR_BADCHANMASK);
            DataToUser(fd_idx, reply);
            return ;
        }
    }
}

/**
 * Command: KICK
 * Parameters: <channel> <user> [<comment>]
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel
 * 3. Reuse the PART method passing the comment as the part message
 */
//void Server::KICK(Command &cmd, int fd_idx) {
//    Channel channel = channel_map.find(cmd.args[1])->second;
//    ChannelUser user = channel.userInChannel(channel, fd_idx);
//    string reply;
//    if (!channel.isUserOperator(user)) {
//        reply = (ERR_CHANOPRIVSNEEDED+cmd.Name()+STR_CHANOPRIVSNEEDED);
//        DataToUser(fd_idx, reply);
//        return ;
//    }
//    this->PART(cmd, )
//}

} // namespace irc
