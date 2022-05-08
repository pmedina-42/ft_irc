#ifndef IRC42_USER_H
#define IRC42_USER_H

#include <string>

namespace irc {
/* TODO: hace falta el belongs fuera del código de prueba? */

class User {
    private:
        std::string _nickName;
        bool belongs;
        int _fd;
        char _mode;

    public:
        User(int, std::string);
        User(int, char*, size_t);
        ~User();

        /* Getters & Setters */
        inline int getFd() { return _fd; }
        inline std::string getNickName() { return _nickName; }
        inline void setNewNick(std::string nick) { _nickName = nick; }
        inline void setMode(char m) { _mode = m; }
        inline char getMode() { return _mode; }

        inline bool stillThere() { return belongs; }
        void leave();
};

}

#endif
