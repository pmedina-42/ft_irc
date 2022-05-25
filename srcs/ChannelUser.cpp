#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

namespace irc {

ChannelUser::ChannelUser(int fd, std::string nick)
	:
		User(fd, nick)
{}

ChannelUser::ChannelUser(int fd, char* data, size_t len)
	:
        User(fd, data, len)
{}

ChannelUser::ChannelUser(User &user)
        :
        User(user)
{}

ChannelUser::~ChannelUser() {}


} // namespace