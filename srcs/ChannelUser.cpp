#include "../includes/ChannelUser.hpp"
#include <algorithm>

using std::string;

namespace irc {

/* Creo que es mejor quereciba un User */
ChannelUser::ChannelUser(int fd)
:
	User(fd),
    mode(' '),
    banned(false)
{}

ChannelUser::ChannelUser(const ChannelUser& other)
:
    User(other),
    mode(other.mode),
    banned(other.banned)
{}

ChannelUser::ChannelUser(const User &other)
:
    User(other),
    mode(' '),
    banned(false)
{}

ChannelUser::~ChannelUser() {}

bool ChannelUser::operator==(ChannelUser const &other) const {
    return ( this->nick == other.nick);
}

} // namespace
