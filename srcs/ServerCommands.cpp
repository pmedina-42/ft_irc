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
    for (UserMap::iterator it = user_map.begin(); it != user_map.end(); it++) {
        if (tools::is_equal(nickname, it->first)) {
            return true;
        }
    }
    return false;
}

/**
 * Command: NICK
 * Parameters: <nickname>
 */
int Server::NICK(Command &cmd, int fd) {

    int size = cmd.args.size();
    if (size != 2) {
        string reply(ERR_NONICKNAMEGIVEN"*"STR_NONICKNAMEGIVEN);
        if (DataToUser(fd, reply) == -1) {
            return ERR_FATAL;
        }
    }
    string nick = cmd.args[1];
    if (nickFormatOk(nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME+nick+STR_ERRONEUSNICKNAME);
        if (DataToUser(fd, reply) == -1) {
            return ERR_FATAL;
        }
        return OK;
    }
    if (nickAlreadyInUse(nick)) {
        string reply(ERR_NICKNAMEINUSE+nick+STR_NICKNAMEINUSE);
        if (DataToUser(fd, reply) == -1) {
            return ERR_FATAL;
        }
        return OK;
    }
    /* case nickname change (fd is recognised, nickname is not) */
    if (!nick_vector[fd].empty()) {
        UserMap::iterator it = user_map.find(nick_vector[fd]);
        it->second.nick = nick;
        nick_vector[fd] = nick;
        return OK;
    }
    /* case new user */
    User new_user(fd, nick);
    user_map.insert(std::make_pair(nick, new_user));
    nick_vector[fd] = nick;
    return OK;
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
int Server::USER(Command &cmd, int fd) {

    int size = cmd.args.size();
    if (size != 5) {
        string reply(ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
        if (DataToUser(fd, reply) == -1) {
            return ERR_FATAL;
        }
        return OK;
    }
    string nick = nick_vector[fd];
    if (nick.empty()) {
        return OK;
    }
    if (!user_map.count(nick)) {
        return OK;
    }
    UserMap::iterator it = user_map.find(nick);
    User user = it->second;
    if (user.registered == true) {
        string reply(ERR_ALREADYREGISTERED""STR_ALREADYREGISTERED);
        if (DataToUser(fd, reply) == -1) {
            return ERR_FATAL;
        }
        return OK;
    }
    user.name = cmd.args[1];
    user.full_name = cmd.args[size - 1];
    user.setPrefixFromHost(hostname);
    user.registered = true;
    string welcome_msg(RPL_WELCOME+user.name+RPL_WELCOME_STR_1+user.prefix);
    if (DataToUser(fd, welcome_msg) == -1) {
        return ERR_FATAL;
    }
    return OK;
}

} // namespace irc