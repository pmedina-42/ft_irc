#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

#include "Types.hpp"

namespace irc {

class User;
/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
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
    bool isInvited(std::string &nick);
    void setUserMode(User& user, char mode);
    void addToWhitelist(std::string &nick);
    bool userIsInChannel(std::string& nick);
    bool isUserOperator(User& user);
    void setUserMode(User &user, std::string mode);
    std::string getNextOpUser(std::string& nick);

    /* ATTRIBUTES */
    NickList users;
    NickList white_list;
    NickList black_list;

    std::string name;
    // TODO: change channel mode to bitmask
    std::string mode;
    unsigned int max_users;
    std::string key;
    /**
     * Channel topic
     */
    std::string topic;

};

} // namespace

#endif
