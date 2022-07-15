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

    /* checkers */
    bool fdExists(int fd);
    bool nickExists(string& nick);
    bool nickFormatOk(string &nickname);
    
    /* accessors */
    User& getUserFromFd(int fd);
    User& getUserFromNick(string& nick);
    int getFdFromNick(string& nick);
    Channel& getChannelFromName(string& name);

    /* interactors */
    void addNewUser(int new_fd);
    void removeUser(int fd_idx);

    void updateUserNick(int fd, string &new_nick, string &new_real_nick);

    void addNickFdPair(string &nick, int fd);
    void removeNickFdPair(string &nick);

    void addFdUserPair(int fd, User& user);
    void removeFdUserPair(int fd);

    void addNewChannel(Channel& new_channel);
    void removeChannel(Channel& channel);

    void debugNickFdMap();
    void debugFdUserMap();
    //void debugChannelMap();
};

}

#endif /* IRC42_IRCDATABASE_H */
