#include "../includes/ChannelUser.hpp"
#include "Tools.hpp"
#include <unistd.h>
#include <algorithm>

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


bool User::hasLeftovers(void) const {
    return !(buffer_size == 0);
}

void User::resetBuffer(void) {
   tools::clean_buffer(buffer, buffer_size);
   buffer_size = 0;
}

void User::addLeftovers(std::string &leftovers) {
    memcpy(buffer, leftovers.c_str(), leftovers.size());
    buffer_size += leftovers.size();
}

string User::BufferToString(void) const {
    return string(buffer, buffer_size);
}


User::~User() {

}


} // namespace
