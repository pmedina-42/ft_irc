#include "../includes/User.hpp"
#include <unistd.h>

namespace irc {

User::User(int fd, std::string nick) : _nickName(nick), belongs(true), _fd(fd) {}

User::User(int fd, char* data, size_t len) : _nickName(data, len), belongs(true), _fd(fd) {}

User::~User() {}


/* CLASS FUNCTIONS */

void User::leave() {
	belongs = false;
	close(_fd);
}

}