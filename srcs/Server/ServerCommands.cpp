#include "Server.hpp"
#include "libft.h"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
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

void Server::sendNeedMoreParamsMsg(string& cmd_name, int fd_idx) {
    string reply(ERR_NEEDMOREPARAMS+cmd_name+STR_NEEDMOREPARAMS);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendNotRegisteredMsg(string &cmd_name, int fd_idx) {
    string reply(ERR_NOTREGISTERED+cmd_name+STR_NOTREGISTERED);
    DataToUser(fd_idx, reply, NUMERIC_REPLY);
}

void Server::sendWelcomeMsg(string& name, string &prefix, int fd_idx) {
    string welcome_msg(RPL_WELCOME+name+RPL_WELCOME_STR_1+prefix);
    DataToUser(fd_idx, welcome_msg, NUMERIC_REPLY);
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

    int fd = fd_manager.getFdFromIndex(fd_idx);

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
        return sendWelcomeMsg(user.name, user.prefix, fd_idx);
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
        return sendNeedMoreParamsMsg(cmd.Name(), fd_idx);
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
    if (size < 2) {
        return sendNeedMoreParamsMsg(cmd.Name(), fd_idx);
    }

    User& user = getUserFromFdIndex(fd_idx);
    if (!user.registered) {
        return sendNotRegisteredMsg(cmd.Name(), fd_idx);
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

} // namespace irc
