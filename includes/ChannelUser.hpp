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

class ChannelUser : public User {

    public:
    ChannelUser(int fd);
    ~ChannelUser();
    bool operator==(ChannelUser const &other) const;

    /* ATTRIBUTES */
    char mode;
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
