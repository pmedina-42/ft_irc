#include "ChannelUser.hpp"
#include <algorithm>

using std::string;

namespace irc {

/* Creo que es mejor quereciba un User */
ChannelUser::ChannelUser(int fd)
:
        User(fd),
        channelMode(' '),
        banned(false)
{}

ChannelUser::ChannelUser(const ChannelUser& other)
:
        User(other),
        channelMode(other.channelMode),
        banned(other.banned)
{}

ChannelUser::ChannelUser(const User &other)
:
        User(other),
        channelMode(' '),
        banned(false)
{}

ChannelUser::~ChannelUser() {}

ChannelUser& ChannelUser::operator=(const ChannelUser &other) {
    if (this != &other) {
        User::operator=(other);
        channelMode = other.channelMode;
        banned = other.banned;
    }
    return *this;
}

ChannelUser& ChannelUser::operator=(const User &other) {
    User::operator=(other);
    return *this;
}

bool ChannelUser::operator==(ChannelUser const &other) const {
    return ( this->nick == other.nick);
}

} // namespace
