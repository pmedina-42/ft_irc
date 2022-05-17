#include "../includes/Channel.hpp"
#include <algorithm>

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(std::string name, User* user) : _name(name) {
    _mode = 0;
    user->_mode = 'o';
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
    user->joinChannel(this);
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
        if (!(*user)->_nickName.compare(name)) {
            if (user == _users.begin()) {
                _users.pop_front();
                std::list<User*>::iterator it = _users.begin();
                while (userInBlackList((*it)->_nickName))
                    it++;
                (*it)->_mode = 'o';
            }
            else
                _users.erase(user);
            (*user)->leaveChannel(this);
            break ;
        }
    }
}

/*
 * Banea a un usuario
 * */
void Channel::banUser(std::string name) {
    _blackList.push_back(name);
}

/*
 * Desbanea a un usuario
 * */
void Channel::unbanUser(std::string name) {

    std::list<std::string>::iterator it = std::find(_blackList.begin(),
                                                    _blackList.end(),
                                                    name);
    if (it != _blackList.end())
        _blackList.erase(it);
}

/*
 * Comprueba si el usuario está en la lista de baneados
 * */
bool Channel::userInBlackList(std::string name) {
    std::list<std::string>::iterator it = std::find(_blackList.begin(),
                                                    _blackList.end(),
                                                    name);
    if (it != _blackList.end())
        return true;
    return false;
}

/*
 * Comprueba si el canal está en modo invitación
 * */
bool Channel::inviteModeOn() {
    if (_mode == 'i')
        return true;
    return false;
}

/*
 * Añade un nuevo usuario a la whitelist 
 * */
void Channel::addToWhitelist(User *user) {
    _whiteList.push_back(user);
}

/*
 * Devuelve true si el usuario está en la whitelist del canal 
 */
bool Channel::isInvited(User *user) {
    std::list<User*>::iterator it = std::find(_whiteList.begin(),
                                              _whiteList.end(),
                                              user);
    if (it != _whiteList.end()) {
        return true;
    }
    return false;
}

/*
 * Función probablemente useless ya que el modo se setea desde el mismo usuario 
 * */
void Channel::setUserMode(std::string name, char mode) {
    if (mode == 'o' || mode == 'v') {
        std::list<User*>::iterator end = _users.end();
        for (std::list<User*>::iterator usr = _users.begin();
            usr != end; usr++)
        {
            if (!(*usr)->_nickName.compare(name)) {
                (*usr)->_mode = mode;
            }
        }
    }
}

} // namespace
