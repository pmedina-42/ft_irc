#ifndef IRC42_IRCDATABASE_H
# define IRC42_IRCDATABASE_H

#include "Types.hpp"
#include <string>

using std::string;


namespace irc {

class IrcDataBase {

    public:
    IrcDataBase(void);
    IrcDataBase(const IrcDataBase& other);
    ~IrcDataBase();

    /* Data Bases */
    ChannelMap channel_map; // <string name, Channel> 
    NickFdMap nick_fd_map;  // <string nick, int fd>
    FdUserMap fd_user_map;  // <int fd, User>

    /* accessors */
    bool fdExistsInServer(int fd);

    User& getUserFromFd(int fd);
    User& getUserFromNick(string& nick);
    int getFdFromNick(string& nick);
    Channel& getChannelFromName(string& name);

    /* interactors */
    void AddNewUser(int new_fd);
    void RemoveUser(int fd_idx);

    /*void AddNewChannel(Channel& new_channel);
    void RemoveChannel(Channel& channel);
    */
};

}

#endif /* IRC42_IRCDATABASE_H */