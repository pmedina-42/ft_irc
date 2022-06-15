#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

namespace irc {

ChannelUser::ChannelUser(int fd, std::string nick)
:
	User(fd, nick)
{}

ChannelUser::ChannelUser(User &user)
:
    User(user)
{}

ChannelUser::~ChannelUser() {}


} // namespace