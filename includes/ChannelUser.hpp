#ifndef IRC42_USER_H
#define IRC42_USER_H

#include <unistd.h>
#include <string.h>
#include <string>
#include "User.hpp"

using std::string;

namespace irc {

class Channel;

class ChannelUser : public User {

    public:
    ChannelUser(int fd, string nick);
    ChannelUser(int fd, char* nick, size_t size);
    ChannelUser(User&);
    ~ChannelUser();

    /* ATTRIBUTES */
    char mode;
    bool banned;
    /* Si vamos a usar un mapa de usuarios, necesitamos conocer el orden
     * en el que se han unido al canal */
    int pos;

    /**
     * Cuando usuario envia comando para hablar
     * en canal, el servidor llama al mapa de canales para
     * averiguar si el usuario está o no en ese canal y hacer
     * lo correspondiente.
     */
};

}

#endif
