#ifndef IRC42_USER_H
#define IRC42_USER_H

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace irc {

class Channel;

class User {

    public:
    User(int fd, string nick);
    User(int fd, char* nick, size_t size);
    ~User();

    void leave();

    void leaveChannel(Channel*);
    void joinChannel(Channel*);

    /* ATTRIBUTES */
    string _nickName;
    string _mask;
    int _fd;
    char _mode;
    vector<Channel*> _channels; /*
                                 * Borrar a futuro. Cuando usuario envia comando para hablar
                                 * en canal, el servidor llama al mapa de canales para 
                                 * averiguar si el usuario está o no en ese canal y hacer
                                 * lo correspondiente.
                                 */
};

}

#endif
