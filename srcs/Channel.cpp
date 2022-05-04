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

/* TODO: añadir controles con los demás modos de canal */
/* 
 * Añadir un usuario a un canal:
 * 1. Se comprueba si el canal está en modo invitación
 * 1.1 Si el usuario no está en la lista de invitados se sale con error (supongo)
 * 2. Se añade el usuario dentro del canal
 * 3. Se añade el canal dentro del usuario
 * */
void Channel::addUser(User* user) {
	if (inviteModeOn() || _mode == 'p') {
		if (!isInvited(user)) {
			/* Mensajito de error por consola no estaria de más */
			return ;
		}
	}
	_users.push_back(user);
	user->joinChannel(*this);
}

/* 
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2.1 Si el usuario a borrar resulta ser el primero de la lista aka el creador:
 * 2.1.1 Se saca de la lista y se pone el rol de operador al siguiente en la lista
 * 2.1.2 Ese siguiente no debe estar en la lista de baneados
 * 2.2 Si es un usuario normal o cualquier otro operador, se borra sin más
 * 3. Se borra el canal de la lista de canales del usuario
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
				(*it)->setMode('o');
			}
			else
				_users.erase(user);
			user->leaveChannel(*this);
			break ;
		}
	}
}

void Channel::banUser(std::string name) {
	_blackList.push_back(name);
}

void Channel::unbanUser(std::string name) {
	std::vector<std::string>::iterator it = _blackList.find(name);
	if (it != _blackList.end())
		_blackList.erase(it);
}

bool Channel::userInBlackList(std::string name) {
	std::list<std::string>::iterator it = find(_blackList.begin(), _blackList.end(), name);
	if (it != _blackList.end())
		return true;
	return false;
}

bool Channel::inviteModeOn() {
	if (_mode == 'i')
		return true;
	return false;
}

bool Channel::isInvited(User *user) {
	std::list<User*>::iterator it = _invited_users.find(user);
	if (it != _invited_users.end()) {
		return true;
	}
	return false;
}

void Channel::setUserMode(std::string name, char mode) {
	if (mode == 'o' || mode == 'v') {
		std::list<User*>::iterator end = _users.end();
		for (std::list<User*>::iterator user = _users.begin(); user != end; user++) {
			if (!(*user)->getNickName().compare(name)) {
				(*user)->setMode(mode);
			}
		}
	}
}
