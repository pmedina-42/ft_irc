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
Channel::Channel(char prefix, string name, ChannelUser& user) : _name(name) {
    _mode = 0;
    _prefix = prefix;
    user.mode = 'o';
    users.insert(pair<string, ChannelUser>(user.getNick(), user));
}

/* 
 * Un canal se borra cuando se queda sin usuarios dentro, 
 * por lo que en principio el destructor estaría vacío 
 * */
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
    if (inviteModeOn() || _mode == 'p') {
        //if (!isInvited(&user)) {
            /* Mensajito de error por consola no estaria de más */
            //return ;
        //}
    }
    users.insert(pair<string, ChannelUser>(user.getNick(), user));
}

/**
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2.1 Si el usuario a borrar resulta ser el primero del mapa aka el creador:
 * 2.1.1 Se saca del mapa y se pone el rol de operador al siguiente en el orden del canal
 * 2.1.2 Ese siguiente no debe estar en la lista de baneados
 * 2.2 Si es un usuario normal o cualquier otro operador, se borra sin más
 * 3. Se borra el canal de la lista de canales del usuario
 */
//void Channel::deleteUser(ChannelUser &user) {
//    map<string, ChannelUser&>::iterator end = users.end();
//    for (map<string, ChannelUser&>::iterator u = users.begin(); u != end; u++) {
//        if (!(*u).second.getNick().compare(user.getNick())) {
//            if ((*u).second.pos == index) {
//                users.erase(u);
//                list<ChannelUser*>::iterator it = users.begin();
//                while (userInBlackList((*it)->_nickName))
//                    it++;
//                (*it)->mode = 'o';
//            }
//            else
//                users.erase(u);
//            break ;
//        }
//    }
//}

/**
 * Banea a un usuario
 */
void Channel::banUser(ChannelUser &user) {
    users.find(user.getNick())->second.banned = true;
}

/**
 * Desbanea a un usuario
 */
void Channel::unbanUser(ChannelUser &user) {
    users.find(user.getNick())->second.banned = false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(ChannelUser &user) {
    return users.find(user.getNick())->second.banned == true ?  true : false;
}

/**
 * Comprueba si el canal está en modo invitación
 */
bool Channel::inviteModeOn() {
    if (_mode == 'i')
        return true;
    return false;
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(ChannelUser &user) {
    whiteList.insert(pair<string, ChannelUser>(user.getNick(), user));
}

/**
 * Devuelve true si el usuario está en la whitelist del canal 
 */
bool Channel::isInvited(ChannelUser &user) {
    ChannelUserMap::iterator it;
    it = whiteList.find(user.getNick());
    if (it != whiteList.end()) {
        return true;
    }
    return false;
}

/**
 * Cambia el modo de un usuario dentro del canal
 */
void Channel::setUserMode(ChannelUser &user, char mode) {
    if (mode == 'o' || mode == 'v') {
        users.find(user.getNick())->second.mode = mode;
    }
}

} // namespace
