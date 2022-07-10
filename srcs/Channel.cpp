#include "Channel.hpp"
#include "ChannelUser.hpp"
#include <algorithm>

#include <Types.hpp>

using std::string;
using std::list;
using std::pair;

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(string name, ChannelUser& user) : _name(name) {
    mode = 0;
    user.mode = 'o';
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
 * 1. Se comprueba si el canal está en modo invitación
 * 1.1 Si el usuario no está en la lista de invitados se sale con error (supongo)
 * 2. Se añade el usuario dentro del canal
 * 3. Se añade el canal dentro del usuario
 */
void Channel::addUser(ChannelUser &user) {
    if (inviteModeOn() || mode == 'p') {
        if (!isInvited(user)) {
            /* Mensajito de error por consola no estaria de más */
            return ;
        }
    }
    users.push_back(user);
}

/**
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2.1 Si el usuario a borrar resulta ser el primero de la lista aka el creador:
 * 2.1.1 Se saca de la lista y se pone el rol de operador al siguiente en la lista
 * 2.1.2 Ese siguiente no debe estar en la lista de baneados
 * 2.2 Si es un usuario normal o cualquier otro operador, se borra sin más
 * 3. Se borra el canal de la lista de canales del usuario
 * */
void Channel::deleteUser(ChannelUser &user) {
    ChannelUserList::iterator end = users.end();
    for (ChannelUserList::iterator u = users.begin(); u != end; u++) {
        if (!u->nick.compare(user.nick)) {
            if (u == users.begin()) {
                users.pop_front();
                ChannelUserList::iterator it = users.begin();
                while (userInBlackList(*it))
                    it++;
                it->mode = 'o';
            }
            else
                users.erase(u);
            break ;
        }
    }
}

/**
 * Banea a un usuario
 */
void Channel::banUser(ChannelUser &user) {
    ChannelUserList::iterator end = users.end();
    ChannelUserList::iterator it = std::find(users.begin(), end, user);
    if (it != end)
        it->banned = true;
}

/**
 * Desbanea a un usuario
 */
void Channel::unbanUser(ChannelUser &user) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, user);
    if (it != end)
        it->banned = false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(ChannelUser &user) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, user);
    return it != end ?  true : false;
}

/**
 * Comprueba si el canal está en modo invitación
 */
bool Channel::inviteModeOn() {
    if (mode == 'i')
        return true;
    return false;
}

/**
 * Check if user has operator mode
 */
bool Channel::isUserOperator(ChannelUser &user) {
    if (user.mode == 'o')
        return true;
    return false;
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(ChannelUser &user) {
    whiteList.insert(pair<string, ChannelUser>(user.nick, user));
}

/**
 * Devuelve true si el usuario está en la whitelist del canal 
 */
bool Channel::isInvited(ChannelUser &user) {
    ChannelUserMap::iterator it;
    it = whiteList.find(user.nick);
    if (it != whiteList.end()) {
        return true;
    }
    return false;
}

/**
 * Cambia el modo de un usuario dentro del canal
 */
void Channel::setUserMode(ChannelUser &user, char mode) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, user);
    if ((mode == 'o' || mode == 'v') && it != end) {
        it->mode = mode;
    }
}

ChannelUser Channel::userInChannel(Channel &channel, int userFd) {
    ChannelUserList::iterator end = channel.users.end();
    for (ChannelUserList::iterator it = channel.users.begin(); it != end; it++) {
        if (it->fd == userFd) {
            return *it;
        }
    }
    return NULL;
}

} // namespace
