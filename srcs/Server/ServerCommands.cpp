#include "Server.hpp"
#include "libft.h"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"

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

bool Server::nickAlreadyInUse(string &nickname) {
    for (FdUserMap::iterator it = fd_user_map.begin();
                        it != fd_user_map.end(); it++)
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
    if (nickAlreadyInUse(nick)) {
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

} // namespace irc
