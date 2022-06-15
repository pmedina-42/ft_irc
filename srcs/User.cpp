#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

namespace irc {

User::User(int fd, std::string nick, string realname)
	:
		nickName(nick),
        fullname(realname),
		fd(fd)
{
    memset(buffer, '\0', SERVER_BUFF_MAX_SIZE);
    buffer_size = 0;
}

User::~User() {

}


} // namespace