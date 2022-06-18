#include "../includes/ChannelUser.hpp"
#include <unistd.h>
#include <algorithm>

namespace irc {

User::User(int fd, std::string nick)
	:
		nick(nick),
		fd(fd),
        registered(false)
{
    memset(buffer, '\0', SERVER_BUFF_MAX_SIZE);
    buffer_size = 0;
}

void User::setPrefixFromHost(std::string &host) {
    prefix = nick + "!" + name + "@" + host;
}

User::~User() {

}


} // namespace