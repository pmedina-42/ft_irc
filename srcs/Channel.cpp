#include "../includes/Channel.hpp"

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 * */
Channel::Channel(std::string name, User* user) : _name(name) {
	_mode = 0;
	user->setMode('o');
	_users.push_front(user);
}

/* 
 * Un canal se borra cuando se queda sin usuarios dentro, 
 * por lo que en principio el destructor estaría vacío 
 * */
Channel::~Channel() {
}


/* CLASS FUNCTIONS */

void Channel::addUser(User* user) {
	_users.push_back(user);
}

/* 
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2.1 Si el usuario a borrar resulta ser el primero de la lista aka el creador:
 * 2.1.1 se saca de la lista y se pone el rol de operador al siguiente en la lista
 * 2.1.2 ese siguiente no debe estar en la lista de baneados
 * 2.2 Si es un usuario normal o cualquier otro operador, se borra sin más
 * */
void Channel::deleteUser(std::string name) {
	std::list<User*>::iterator end = _users.end();
	for (std::list<User*>::iterator user = _users.begin(); user != end; user++) {
		if (!(*user)->getNickName().compare(name)) {
			if (user == _users.begin()) {
				_users.pop_front();
				std::list<User*>::iterator it = _users.begin();
				while (userInBlackList((*it)->getNickName()))
					it++;
				_users.front()->setMode('o');
			}
			else
				_users.erase(user);
		}
	}
}

void Channel::banUser(std::string name) {
	_blackList.push_back(name);
}

bool Channel::userInBlackList(std::string name) {
	std::list<std::string>::iterator it = find(_blackList.begin(), _blackList.end(), name);
	if (it != _blackList.end())
		return true;
	return false;
}
