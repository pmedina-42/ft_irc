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
        real_nick(),
        nick(),
        name(),
        full_name(),
        prefix(),
        mask(),
        server_mode(),
        afk_msg(),
        connection_pass(),
        channel_mode(),
        banned(false),
        ch_name_mask_map(),
        buffer_size(0),
        registered(false),
        on_pong_hold(false),
        last_received(time(NULL)),
        ping_send_time(0),
        ping_str()
{
    ft_memset(buffer, '\0', BUFF_MAX_SIZE);
}

User::User(const User &other)
:
    fd(other.fd),
    real_nick(other.real_nick),
    nick(other.nick),
    name(other.name),
    full_name(other.full_name),
    prefix(other.prefix),
    mask(other.mask),
    server_mode(other.server_mode),
    afk_msg(other.afk_msg),
    connection_pass(other.connection_pass),
    channel_mode(other.channel_mode),
    banned(other.banned),
    ch_name_mask_map(other.ch_name_mask_map),
    buffer_size(other.buffer_size),
    registered(other.registered),
    on_pong_hold(other.on_pong_hold),
    last_received(other.last_received),
    ping_send_time(other.ping_send_time),
    ping_str(other.ping_str)
{
    ft_memset(buffer, '\0', BUFF_MAX_SIZE);
    if (other.buffer_size > 0) {
        ft_memcpy(buffer, other.buffer, other.buffer_size);
    }
}

User& User::operator=(const User& other) {
    if (this != &other) {
        fd = other.fd;
        real_nick = other.real_nick;
        nick = other.nick;
        name = other.name;
        full_name = other.full_name;
        prefix = other.prefix;
        mask = other.mask;
        server_mode = other.server_mode;
        afk_msg = other.afk_msg;
        connection_pass = other.connection_pass;
        channel_mode = other.channel_mode;
        banned = other.banned;
        ch_name_mask_map = other.ch_name_mask_map;
        ft_memset(buffer, '\0', BUFF_MAX_SIZE);
        if (other.buffer_size > 0) {
            ft_memcpy(buffer, other.buffer, other.buffer_size);
        }
        buffer_size = other.buffer_size;
        registered = other.registered;
        on_pong_hold = other.on_pong_hold;
        last_received = other.last_received;
        ping_send_time = other.ping_send_time;
        ping_str = other.ping_str;
    }
    return *this;
}

/*
 * A user is equal to some other if their nick are strcase equal.
 */
bool User::operator==(User const &other) const {
    return ( this->nick == other.nick);
}

/*
 * Prefix from host is done with the real nick ! The other
 * (nick) is always in uppercase.
 */
void User::setPrefixFromHost(std::string &host) {
    prefix = real_nick + "!" + name + "@" + host;
}

// TODO
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
    ft_memcpy(buffer + buffer_size, leftovers.c_str(), leftovers.size());
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

User::~User() {
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
