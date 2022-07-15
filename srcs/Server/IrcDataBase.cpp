#include "Server/IrcDataBase.hpp"
#include "User.hpp"
#include "Log.hpp"
#include "Channel.hpp"
#include "libft.h"

namespace irc {

IrcDataBase::IrcDataBase(void) {
}

IrcDataBase::~IrcDataBase() {
}

IrcDataBase::IrcDataBase(const IrcDataBase& other)
:
    channel_map(other.channel_map),
    nick_fd_map(other.nick_fd_map),
    fd_user_map(other.fd_user_map)
{}

void IrcDataBase::addNewUser(int new_fd) {
    /* case server is full of users */
    if (new_fd == -1) {
        return ;
    }
    User user(new_fd);
    addFdUserPair(new_fd, user);
}


void IrcDataBase::removeUser(int fd) {
    
    LOG(DEBUG);
    LOG(DEBUG) << "REMOVING FD " << fd;
    debugNickFdMap();
    debugFdUserMap();

    User &user = getUserFromFd(fd);
    LOG(INFO) << "User " << user << " removed"; 
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        LOG(DEBUG) << "removing user with nick " << user.real_nick << " from nickfdmap";
        removeNickFdPair(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    removeFdUserPair(fd);

    LOG(DEBUG);
    LOG(DEBUG) << "AFTER REMOVING FD " << fd;
    debugNickFdMap();
    debugFdUserMap();
}

void IrcDataBase::updateUserNick(int fd, string &new_nick,
                                 string &new_real_nick)
{
    User& user = getUserFromFd(fd);
    removeNickFdPair(user.nick);
    addNickFdPair(new_nick, fd);
    user.nick = new_nick;
    user.real_nick = new_real_nick;
}

void IrcDataBase::addFdUserPair(int fd, User& user) {
    fd_user_map.insert(std::make_pair(fd, user));
}

void IrcDataBase::removeFdUserPair(int fd) {
    fd_user_map.erase(fd);
}

void IrcDataBase::addNickFdPair(string &nick, int fd) {
    nick_fd_map.insert(std::make_pair(nick, fd));
}

void IrcDataBase::removeNickFdPair(string &nick) {
    nick_fd_map.erase(nick);
}

void IrcDataBase::addNewChannel(Channel& new_channel) {
    channel_map.insert(std::make_pair(new_channel.name, new_channel));
}

void IrcDataBase::removeChannel(Channel& channel) {
    channel_map.erase(channel_map.find(channel.name));
}

bool IrcDataBase::fdExists(int fd) {
    return fd_user_map.count(fd);
}

bool IrcDataBase::nickExists(string &nick) {
    return nick_fd_map.count(nick);
}

/* See 
 * https://forums.mirc.com/ubbthreads.php/topics/186181/nickname-valid-characters */
bool IrcDataBase::nickFormatOk(string &nickname) {

    if (nickname.empty() || nickname.length() > NAME_MAX_SIZE) {
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

User& IrcDataBase::getUserFromFd(int fd) {
    FdUserMap::iterator it = fd_user_map.find(fd);
    return it->second;
}

int IrcDataBase::getFdFromNick(string& nickname) {
    NickFdMap::iterator it = nick_fd_map.find(nickname);
    return it->second;
}

User& IrcDataBase::getUserFromNick(string& nickname) {
    int fd = getFdFromNick(nickname);
    FdUserMap::iterator it = fd_user_map.find(fd);
    return it->second;
}

Channel& IrcDataBase::getChannelFromName(string& name) {
    ChannelMap::iterator it = channel_map.find(name);
    return it->second;
}

void IrcDataBase::debugFdUserMap(void) {
    for(FdUserMap::const_iterator it = fd_user_map.begin();
        it != fd_user_map.end(); ++it)
    {
        LOG(DEBUG) << "[FD USER MAP] first : " << it->first << ", second : " << &(it->second);
    }
}

void IrcDataBase::debugNickFdMap(void) {
    for(NickFdMap::const_iterator it = nick_fd_map.begin();
        it != nick_fd_map.end(); ++it)
    {
        LOG(DEBUG) << "[NICK FD MAP] first : " << it->first << ", second : " << &(it->second);
    }
}

} // namespace
