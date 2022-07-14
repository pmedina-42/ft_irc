#include "ChannelUser.hpp"
#include <algorithm>

using std::string;

namespace irc {

ChannelUser::ChannelUser(const ChannelUser& other)
:
        user(other.user),
        channel_mode(other.channel_mode),
        banned(other.banned)
{}

ChannelUser::ChannelUser(User &other)
:
        user(other),
        channel_mode(' '),
        banned(false)
{}

ChannelUser::~ChannelUser() {}

ChannelUser& ChannelUser::operator=(const ChannelUser &other) {
    if (this != &other) {
        user = other.user;
        channel_mode = other.channel_mode;
        banned = other.banned;
    }
    return *this;
}

ChannelUser& ChannelUser::operator=(const User &other) {
    user = other;
    return *this;
}

bool ChannelUser::operator==(ChannelUser const &other) const {
    return ( this->user.nick == other.user.nick);
}

} // namespace
