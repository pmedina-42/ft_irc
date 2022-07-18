#ifndef IRC42_IRCDATABASE_H
# define IRC42_IRCDATABASE_H

#include "Types.hpp"
#include <string>

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
    bool nickExists(std::string& nick);
    bool nickFormatOk(std::string &nickname);
    
    /* accessors */
    User& getUserFromFd(int fd);
    User& getUserFromNick(std::string& nick);
    int getFdFromNick(std::string& nick);
    Channel& getChannelFromName(std::string& name);

    /* interactors */
    void addNewUser(int new_fd);
    void removeUser(int fd_idx);

    void updateUserNick(int fd, std::string &new_nick, std::string &new_real_nick);

    void addNickFdPair(std::string &nick, int fd);
    void removeNickFdPair(std::string &nick);

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
