#ifndef USER_H
#define USER_H

#include "Channel.hpp"

#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>

/* TODO: hace falta el belongs fuera del código de prueba? */

class Channel;

class User {
    private:
        std::string _nickName;
		std::string _mask;
        bool belongs;
        int _fd;
        char _mode;
		std::vector<Channel*> _channels;

    public:
        User(int, std::string);
        User(int, char*, size_t);
        ~User();

        /* Getters & Setters */
        inline int getFd() { return _fd; }
        inline std::string getNickName() { return _nickName; }
        inline void setNewNick(std::string nick) { _nickName = nick; }
        inline std::string getMask() { return _mask; }
        inline void setMask(std::string mask) { _mask = mask; }
        inline void setMode(char m) { _mode = m; }
        inline char getMode() { return _mode; }
		inline std::vector<Channel*> getChannels() { return _channels; }

        inline bool stillThere() { return belongs; }
        void leave();

		void leaveChannel(Channel*);
};

#endif
