#ifndef IRC42_IUSER_H
#define IRC42_IUSER_H

#include <unistd.h>
#include <string.h>
#include <string>

using std::string;

namespace irc {

class User {

    public:
    User(int fd, string nick);
    User(int fd, char* nick, size_t size);
    virtual ~User();

    /* ATTRIBUTES */
    string _nickName;
    string _mask;
    int _fd;

    inline string getNick() { return _nickName; }
};

}

#endif
