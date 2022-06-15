#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

using std::string;

namespace irc {

/* Creo que es mejor quereciba un User */
ChannelUser::ChannelUser(int fd, string nick)
:
	User(fd, nick)
{}

ChannelUser::~ChannelUser() {}

} // namespace