#include "Tools.hpp"
#include <unistd.h>
#include <algorithm>
#include <string.h>
#include "User.hpp"
#include "libft.h"

namespace irc {

User::User(int fd)
:
		fd(fd),
        buffer_size(0),
        registered(false),
        last_received(time(NULL)),
        ping_send_time(0)
{
    nick = "";
    name = "";
    full_name = "";
    prefix = "";
    mask = "";
    on_pong_hold = false;
    ping_str = "";
    ft_memset(buffer, '\0', BUFF_MAX_SIZE);
}

User::User(const User &other)
:
    fd(other.fd),
    nick(other.nick),
    name(other.name),
    full_name(other.full_name),
    prefix(other.prefix),
    mask(other.mask),
    buffer_size(other.buffer_size),
    on_pong_hold(other.on_pong_hold),
    last_received(other.last_received),
    ping_send_time(other.ping_send_time),
    ping_str(other.ping_str)
{
    ft_memset(buffer, '\0', BUFF_MAX_SIZE);
    if (other.buffer_size > 0) {
        memcpy(buffer, other.buffer, other.buffer_size);
    }
}

User& User::operator=(const User& other) {
    if (this != &other) {
        fd = other.fd;
        nick = other.nick;
        name = other.name;
        full_name = other.full_name;
        prefix = other.prefix;
        ft_memset(buffer, '\0', BUFF_MAX_SIZE);
        if (other.buffer_size > 0) {
            memcpy(buffer, other.buffer, other.buffer_size);
        }
        buffer_size = other.buffer_size;
        /* pign stuff (important) */
        on_pong_hold = other.on_pong_hold;
        last_received = other.last_received;
        ping_send_time = other.ping_send_time;
        ping_str = other.ping_str;
    }
    return *this;
}

bool User::operator==(User const &other) const {
    return ( this->nick == other.nick);
}

void User::setPrefixFromHost(std::string &host) {
    prefix = real_nick + "!" + name + "@" + host;
}

void User::setChannelMask(string& name, char mode) {
    (void)name;
    (void)mode;
}

bool User::hasLeftovers(void) const {
    return !(buffer_size == 0);
}

void User::resetBuffer(void) {
   tools::cleanBuffer(buffer, buffer_size);
   buffer_size = 0;
}

void User::addLeftovers(std::string &leftovers) {
    memcpy(buffer + buffer_size, leftovers.c_str(), leftovers.size());
    buffer_size += leftovers.size();
}

string User::BufferToString(void) const {
    return string(buffer, buffer_size);
}

time_t User::getLastMsgTime(void) {
    return last_received;
}

time_t User::getPingTime(void) {
    return ping_send_time;
}

bool User::isOnPongHold(void) {
    return on_pong_hold;
}

void User::resetPingStatus(void) {
    on_pong_hold = false;
    ping_str = "";
    last_received = time(NULL);
}

void User::updatePingStatus(string &random) {
    ping_str = random;
    on_pong_hold = true;
    ping_send_time = time(NULL);
}

bool User::isResgistered(void) {
    return registered;
}

/* 
 * We use references, not pointers in this IRC. So memseting
 * to 0 all space used by user class helps with weird errors
 * that occurr when a stack address that was used by User1,
 * once it is deleted is occupied by User2 */
User::~User() {
    ft_memset(this, '\0', sizeof(User));
}

} // namespace

std::ostream& operator<<(std::ostream &o, const irc::User &rhs ) {
    if (rhs.registered) {
        o << rhs.prefix;
    } else {
        o << "John Doe with fd " << rhs.fd;
    }
    return o;
}
