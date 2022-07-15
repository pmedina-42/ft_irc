#include "Server/IrcDataBase.hpp"
#include "User.hpp"
#include "Log.hpp"
#include "Channel.hpp"

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

void IrcDataBase::AddNewUser(int new_fd) {
    /* case server is full of users */
    if (new_fd == -1) {
        return ;
    }
    User user(new_fd);
    fd_user_map.insert(std::make_pair(new_fd, user));
}


void IrcDataBase::RemoveUser(int fd) {
    User &user = getUserFromFd(fd);
    LOG(INFO) << "User " << user << " removed"; 
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        nick_fd_map.erase(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    fd_user_map.erase(fd); 
}

bool IrcDataBase::fdExistsInServer(int fd) {
    return fd_user_map.count(fd);
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
    FdUserMap::iterator it_2 = fd_user_map.find(fd);
    return it_2->second;
}

Channel& IrcDataBase::getChannelFromName(string& name) {
    ChannelMap::iterator it = channel_map.find(name);
    return it->second;
}

}