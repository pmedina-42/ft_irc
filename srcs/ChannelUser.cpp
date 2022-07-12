#include "../includes/ChannelUser.hpp"
#include <algorithm>

using std::string;

namespace irc {

/* Creo que es mejor quereciba un User */
ChannelUser::ChannelUser(int fd)
:
	User(fd)
{
    mode = ' ';
}

ChannelUser::~ChannelUser() {}

bool ChannelUser::operator==(ChannelUser const &other) const {
    return ( this->nick == other.nick);
}

} // namespace
