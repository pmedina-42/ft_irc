#include "Channel.hpp"
#include "ChannelUser.hpp"
#include <algorithm>
#include "Log.hpp"

#include <Types.hpp>

using std::string;
using std::list;
using std::pair;

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(string name, ChannelUser& ch_user) : name(name) {
    mode = "";
    ch_user.channel_mode = 'o';
    users.push_back(ch_user);
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
void Channel::addUser(ChannelUser &ch_user) {
    users.push_back(ch_user);
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
void Channel::deleteUser(ChannelUser &ch_user) {
    ChannelUserList::iterator end = users.end();
    for (ChannelUserList::iterator u = users.begin(); u != end; u++) {
        if (!u->user.nick.compare(ch_user.user.nick)) {
            if (u == users.begin()) {
                ChannelUserList::iterator it = users.begin();
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
void Channel::banUser(ChannelUser &ch_user) {
    ChannelUserList::iterator end = users.end();
    ChannelUserList::iterator it = std::find(users.begin(), end, ch_user);
    if (it != end)
        it->banned = true;
}

/**
 * Desbanea a un usuario
 */
void Channel::unbanUser(ChannelUser &ch_user) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, ch_user);
    if (it != end)
        it->banned = false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(ChannelUser &ch_user) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, ch_user);
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
bool Channel::isUserOperator(ChannelUser &ch_user) {
    return (ch_user.channel_mode == 'o');
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(ChannelUser &ch_user) {
    white_list.insert(std::make_pair(ch_user.user.nick, ch_user));
}

/**
 * Devuelve true si el usuario está en la white_list del canal 
 */
bool Channel::isInvited(ChannelUser &ch_user) {
    return (white_list.find(ch_user.user.nick) != white_list.end());
}

/**
 * Cambia el modo de un usuario dentro del canal
 */
void Channel::setUserMode(ChannelUser &ch_user, char mode) {
    list<ChannelUser>::iterator end = users.end();
    list<ChannelUser>::iterator it = std::find(users.begin(), end, ch_user);
    if ((mode == 'o' || mode == 'v') && it != end) {
        it->channel_mode = mode;
    }
}

ChannelUser& Channel::userInChannel(int userFd) {
    ChannelUserList::iterator end = users.end();
    for (ChannelUserList::iterator it = users.begin(); it != end; it++) {
        if (it->user.fd == userFd) {
            return *it;
        }
    }
    return *end;
}

/**
* Check if user has operator channel_mode
*/
ChannelUser& Channel::findUserByNick(string nick) {
    ChannelUserList::iterator it;
    ChannelUserList::iterator end = users.end();
    for (it = users.begin(); it != end; it++) {
        if (!it->user.nick.compare(nick)) {
            return *it;
        }
    }
    return *end;
}

} // namespace
