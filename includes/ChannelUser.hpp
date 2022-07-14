#ifndef IRC42_CHANNELUSER_H
#define IRC42_CHANNELUSER_H

#include <unistd.h>
#include <string.h>
#include <string>
#include "User.hpp"

using std::string;

namespace irc {

class Channel;
class User;

class ChannelUser {

    public:
    ChannelUser(const ChannelUser& other);
    ChannelUser(User &other);
    ~ChannelUser();
    ChannelUser& operator=(const ChannelUser &other);
    ChannelUser& operator=(const User &other);
    bool operator==(ChannelUser const &other) const;

    /* ATTRIBUTES */
    User& user;
    char channel_mode;
    bool banned;

    /**
     * Cuando usuario envia comando para hablar
     * en canal, el servidor llama al mapa de canales para
     * averiguar si el usuario está o no en ese canal y hacer
     * lo correspondiente.
     */
};

}

#endif
