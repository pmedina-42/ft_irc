#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

#include "Types.hpp"

namespace irc {

class User;

class Channel {

    typedef std::list<std::string> NickList;


    public:
    Channel(std::string name, User& user);
    ~Channel();

    /* Class functions */
    void addUser(User& user);
    void deleteUser(User& user);
    void banUser(User& user);
    void unbanUser(User& user);
    bool userInBlackList(std::string nick);
    /* Esta función devuelve true si el canal está en modo invitación */
    bool inviteModeOn();
    bool keyModeOn();
    bool topicModeOn();
    bool banModeOn();
    bool moderatedModeOn();
    bool isInvited(std::string &nick);
    void addToWhitelist(std::string &nick);
    bool userIsInChannel(std::string& nick);
    bool isUserOperator(User& user);
    std::string getNextOpUser(std::string& nick);
    void addMode(int bits);
    void deleteMode(int bits);
    std::string getModeStr();
    void updateUserNick(std::string &nick);

    /* ATTRIBUTES */
    NickList users;
    NickList white_list;
    NickList black_list;

    std::string name;
    unsigned char mode;
    unsigned int max_users;
    std::string key;
    /**
     * Channel topic
     */
    std::string topic;

};

} // namespace

#endif
