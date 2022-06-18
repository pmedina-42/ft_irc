#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>
#include <iostream>

namespace irc {

User::User(int fd)
:
		fd(fd),
        buffer_size(0),
        registered(false)
{
    nick = "";
    name = "";
    full_name = "";
    prefix = "";
    memset(buffer, '\0', SERVER_BUFF_MAX_SIZE);
}

User::User(const User &other) {
    if (this != &other) {
        fd = other.fd;
        nick = other.nick;
        name = other.name;
        full_name = other.full_name;
        prefix = other.prefix;
        memset(buffer, '\0', SERVER_BUFF_MAX_SIZE);
        if (other.buffer_size > 0) {
            memcpy(buffer, other.buffer, other.buffer_size);
        }
        buffer_size = other.buffer_size;
    }
}

User& User::operator=(const User& other) {
    if (this != &other) {
        fd = other.fd;
        nick = other.nick;
        name = other.name;
        full_name = other.full_name;
        prefix = other.prefix;
        memset(buffer, '\0', SERVER_BUFF_MAX_SIZE);
        if (other.buffer_size > 0) {
            memcpy(buffer, other.buffer, other.buffer_size);
        }
        buffer_size = other.buffer_size;
    }
    return *this;
}

void User::setPrefixFromHost(std::string &host) {
    prefix = nick + "!" + name + "@" + host;
}

User::~User() {

}


} // namespace