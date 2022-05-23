#ifndef IRC42_USER_H
#define IRC42_USER_H

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>

namespace irc {

class Channel;

class User {

    public:
        User(int fd, std::string nick);
        User(int fd, char* nick, size_t size);
        ~User();

        void leave();

        void leaveChannel(Channel*);
        void joinChannel(Channel*);

        /* ATTRIBUTES */
        std::string _nickName;
        std::string _mask;
        int _fd;
        char _mode;
        std::vector<Channel*> _channels;
};

}

#endif
