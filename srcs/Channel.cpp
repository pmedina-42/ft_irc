#include "Channel.hpp"
#include <algorithm>
#include "Log.hpp"
#include "User.hpp"
#include "Tools.hpp"

#include <Types.hpp>

using std::string;

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(string name, User& user) : name(name) {
    mode = "";
    user.setChannelMask(name, 'o');
    users.push_back(user);
}

/**
 * Un canal se borra cuando se queda sin usuarios dentro, 
 * por lo que en principio el destructor estaría vacío 
 */
Channel::~Channel() {
}


/* CLASS FUNCTIONS */

/* TODO: añadir controles con los demás modos de canal */
/**
 * Añadir un usuario a un canal:
 * 1. Se comprueba la disponibilidad del canal a nivel de comando, antes de llamar esta funcion
 * 2. Se añade el usuario a la lista de usuarios
 */
void Channel::addUser(User &user) {
    users.push_back(user);
}

/** TODO : 
 *  funcion que gestione asignar un nuevo operador :
 *  busque primero algun otro usuario con 'o', y en caso de
 *  que no que itere la lista hasta que de con el heredero correcto. 
 * 
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2.1 Si el usuario a borrar resulta ser el primero de la lista aka el creador:
 * 2.1.1 Se saca de la lista y se pone el rol de operador al siguiente en la lista
 * 2.1.2 Ese siguiente no debe estar en la lista de baneados
 * 2.2 Si es un usuario normal o cualquier otro operador, se borra sin más
 * 3. Se borra el canal de la lista de canales del usuario
 * */
void Channel::deleteUser(User &user) {
    UserList::iterator end = users.end();
    for (UserList::iterator u = users.begin(); u != end; u++) {
        if (!tools::isEqual(u->nick, user.nick)) {
            /* If user is at beggining of list, then it is an operator */
            if (u == users.begin()) {
                UserList::iterator it = users.begin();
                while (userInBlackList(*it))
                    it++;
                it->channel_mode = 'o';
                users.erase(u);
            } else {
                users.erase(u);
            }
            break ;
        }
    }
}

/**
 * Banea a un usuario
 */
void Channel::banUser(User &user) {
    UserList::iterator end = users.end();
    UserList::iterator it = std::find(users.begin(), end, user);
    if (it != end)
        it->banned = true;
}

/**
 * Desbanea a un usuario
 */
void Channel::unbanUser(User &user) {
    UserList::iterator end = users.end();
    UserList::iterator it = std::find(users.begin(), end, user);
    if (it != end)
        it->banned = false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(User &user) {
    UserList::iterator end = users.end();
    UserList::iterator it = std::find(users.begin(), end, user);
    return it != end ?  true : false;
}

/**
 * Comprueba si el canal está en modo invitación
 */
bool Channel::inviteModeOn() {
    return (mode.find("i") != string::npos);
}

/**
 * Comprueba si el canal está en modo contraseña
 */
bool Channel::keyModeOn() {
    return (mode.find("k") != string::npos);
}

/**
 * Check if user has operator channel_mode
 */
bool Channel::isUserOperator(User &user) {
    return (user.channel_mode == 'o');
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(User &user) {
    white_list.insert(std::make_pair(user.nick, user));
}

/**
 * Devuelve true si el usuario está en la white_list del canal 
 */
bool Channel::isInvited(User &user) {
    return (white_list.find(user.nick) != white_list.end());
}

/**
 * Cambia el modo de un usuario dentro del canal
 */
void Channel::setUserMode(User &user, char mode) {
    /*UserList::iterator end = users.end();
    UserList::iterator it = std::find(users.begin(), end, user);
    if ((mode == 'o' || mode == 'v') && it != end) {
        it->channel_mode = mode;
    }*/
    user.channel_mode = mode;
}

bool Channel::userIsInChannel(int userFd) {
    for (UserList::iterator it = users.begin(); it != users.end(); it++) {
        if (it->fd == userFd) {
            return true;
        }
    }
    return false;
}

bool Channel::userIsInChannel(string& nick) {
    for (UserList::iterator it = users.begin(); it != users.end(); it++) {
        if (tools::isEqual(it->nick, nick)) {
            return true;
        }
    }
    return false; 
}

User& Channel::getUserFromFd(int fd) {
    UserList::iterator end = users.end();
    for (UserList::iterator it = users.begin(); it != end; it++) {
        if (it->fd == fd) {
            return *it;
        }
    }
    return *end; // it never gets here
}

User& Channel::getUserFromNick(string& nick) {
    UserList::iterator end = users.end();
    for (UserList::iterator it = users.begin(); it != end; it++) {
        if (tools::isEqual(it->nick, nick)) {
            return *it;
        }
    }
    return *end; // it never gets here
    
}

/**
* Check if user has operator channel_mode
*/
User& Channel::findUserByNick(string& nick) {
    UserList::iterator it;
    UserList::iterator end = users.end();
    for (it = users.begin(); it != end; it++) {
        if (tools::isEqual(it->nick, nick)) {
            return *it;
        }
    }
    return *end;
}

} // namespace
