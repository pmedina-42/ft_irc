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

/**
 * Command: NICK
 * Parameters: <nickname>
 */
void Server::NICK(Command &cmd, int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;

    int size = cmd.args.size();
    /* NICK arguments incorrect */
    if (size != 2) {
        string reply(ERR_NONICKNAMEGIVEN "*" STR_NONICKNAMEGIVEN);
        DataToUser(fd_idx, reply);
        return ;
    }
    string nick = cmd.args[1];
    /* case forbidden characters are found / incorrect length */
    if (nickFormatOk(nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME+nick+STR_ERRONEUSNICKNAME);
        DataToUser(fd_idx, reply);
        return ;
    }
    /* case nickname is equal to some other in the server
     * (ignoring upper/lower case) */
    if (nickAlreadyInUse(nick)) {
        string reply(ERR_NICKNAMEINUSE+nick+STR_NICKNAMEINUSE);
        DataToUser(fd_idx, reply);
        return ;
    }
    FdUserMap::iterator it = fd_user_map.find(fd);
    User& user = it->second;
    /* case nickname change (fd is recognised, nickname is not) */
    if (!user.nick.empty()) {
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
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void Server::USER(Command &cmd, int fd_idx) {

    int fd = fd_manager.fds[fd_idx].fd;

    int size = cmd.args.size();
    /* case arguments make no sense */
    if (size != 5) {
        string reply(ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
        DataToUser(fd_idx, reply);
        return;
    }
    FdUserMap::iterator it = fd_user_map.find(fd);
    User& user = it->second;
    /* if nickname is not definedf, ignore command */
    if (user.nick.empty()) {
        return ;
    }
    /* case user already sent a valid USER comand */
    if (user.registered == true) {
        string reply(ERR_ALREADYREGISTERED "" STR_ALREADYREGISTERED);
        DataToUser(fd_idx, reply);
        return ;
    }
    /* generic case (USER comand after NICK for registration) */
    user.name = cmd.args[1];
    user.full_name = cmd.args[size - 1];
    user.setPrefixFromHost(hostname);
    user.registered = true;
    string welcome_msg(RPL_WELCOME+user.name+RPL_WELCOME_STR_1+user.prefix);
    DataToUser(fd_idx, welcome_msg);
}

/**
 * Command: JOIN
 * Parameters: <channel> [<channel>] [<key>]
 */
void Server::JOIN(Command &cmd, int fd_idx) {
    int size = cmd.args.size();

}

/**
 * Command: PART
 * Parameters: <channel> [, <channel>] [<partMessage>]
 * 1. If command PART has no arguments, error message is returned
 * 2. Iterate every channel in the channel_map (check by it's mask)
 * 3. In each channel, check if it exists and if user belongs to channel
 *  If not, error message is returned
 */
void Server::PART(Command &cmd, int fd_idx) {
        int size = cmd.args.size();
        if (size < 2) {
            string reply = (ERR_NEEDMOREPARAMS+cmd.Name()+STR_NEEDMOREPARAMS);
            DataToUser(fd_idx, reply);
            return ;
        }
        vector<string>::iterator end = cmd.args.end();
        vector<string>::iterator it;
        for (it = cmd.args.begin() + 1; it < end; it++) {
            if (tools::starts_with_mask(it[0])) {
                if (!channel_map.count(*it)) {
                    string reply = (ERR_NOSUCHCHANNEL+cmd.Name()+STR_NOSUCHCHANNEL);
                    DataToUser(fd_idx, reply);
                    return ;
                }
                Channel channel = channel_map.find(*it)->second;
                ChannelUser user = channel.userInChannel(channel, fd_idx);
                if (user == NULL) {
                    string reply = (ERR_NOTONCHANNEL+cmd.Name()+STR_NOTONCHANNEL);
                    DataToUser(fd_idx, reply);
                    return ;
                }
                channel.deleteUser(user);
            } else {
                break ;
            }
        }
        if (it != end) {
            // TODO: mandar mensaje en los canales
        } else {

        }
    }

} // namespace irc
