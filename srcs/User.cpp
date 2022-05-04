#include "../includes/User.hpp"

User::User(int fd, std::string nick) : _nickName(nick), belongs(true), _fd(fd) {}

User::User(int fd, char* data, size_t len) : _nickName(data, len), belongs(true), _fd(fd) {}

User::~User() {}


/* CLASS FUNCTIONS */

void User::leave() {
	belongs = false;
	close(_fd);
}

/* 
 * Antes de permitir que un usuario se una a un canal,
 * hay que comprobar desde el mismo que puede hacerlo 
 * */
void User::joinChannel(Channel *channel) {
	_channels.push_back(channel);
}

/* 
 * Abandonar canal
 * 1. Iterar hasta encontrar el canal que el usuario va a abandonar
 * 2. Una vez encontrado, se borra el canal de la lista 
 * */
void User::leaveChannel(Channel *channel) {
	std::vector<Channel*>::iterator it = _channels.find(channel);
	if (it != _channels.end())
		_channels.erase()
}
