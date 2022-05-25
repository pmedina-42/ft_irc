#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

namespace irc {

User::User(int fd, std::string nick)
	:
		_nickName(nick),
		_fd(fd)
{}

User::User(int fd, char* data, size_t len)
	:
		_nickName(data, len),
		_fd(fd)
{}

User::~User() {}


} // namespace