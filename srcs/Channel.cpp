#include "Channel.hpp"

Channel::Channel(std::string name, User* user) : _name(name) {
	/* Aquí habría que setearle el rol de operador al creador del canal */
	_users.push_front(user);
}
