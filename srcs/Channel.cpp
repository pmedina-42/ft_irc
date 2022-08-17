#include "Channel.hpp"
#include <algorithm>
#include "Log.hpp"
#include "User.hpp"
#include "Tools.hpp"

#include <Types.hpp>

using std::string;
using std::list;

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(string name, User& user) : name(name), mode(0) {
    users.push_back(user.nick);
    addMode(CH_TOP);
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
    users.push_back(user.nick);
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
 * 
 */
void Channel::deleteUser(User &user) {
    NickList::iterator end = users.end();
    for (NickList::iterator u = users.begin(); u != end; u++) {
        if (!u->compare(user.nick)) {
            /* Search for first non-blacklist user */
            if (u == users.begin() && users.size() > 1) {
                NickList::iterator it = ++users.begin();
                while (userInBlackList(*it) && it != end)
                    it++;
                /* no users outside blacklist, delete channel */
                if (it == end) {
                    users.clear();
                }
            }
            users.erase(u);
            break ;
        }
    }
}

string Channel::getNextOpUser(string &nick) {
    NickList::iterator end = users.end();
    for (NickList::iterator u = users.begin(); u != end; u++) {
        LOG(DEBUG) << "Entra con " << nick << ", compara con " << *u << ", el primero es " << *users.begin();
        if (!u->compare(nick) && u == users.begin() && users.size() > 1) {
            NickList::iterator it = ++users.begin();
            while (userInBlackList(*it) && it != end)
                    it++;
            if (it != end) {
                LOG(DEBUG) << "Devuelve siguiente usuario " << *it;
                return (*it);
            }
        }
    }
    return "";
}

/**
 * Banea a un usuario
 */
void Channel::banUser(User &user) {
    black_list.push_back(user.nick);
}

/**
 * Desbanea a un usuario
 */
void Channel::unbanUser(User &user) {
    list<string>::iterator end = black_list.end();
    list<string>::iterator it = std::find(black_list.begin(), end, user.nick);
    if (it != end)
        black_list.erase(it);
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(string nick) {
    list<string>::iterator end = black_list.end();
    list<string>::iterator it = std::find(black_list.begin(), end, nick);
    return (it != end);
}

/**
 * Comprueba si el canal está en modo invitación
 */
bool Channel::inviteModeOn() {
    return ((mode & 0x08) >> CH_INV);
}

/**
 * Comprueba si el canal está en modo contraseña
 */
bool Channel::keyModeOn() {
        return (((mode & 0x10) >> CH_PAS));
}

/**
 * Comprueba si el canal está en modo baneos
 */
bool Channel::banModeOn() {
    return (((mode & 0x20) >> CH_BAN));
}

/**
 * Comprueba si el canal está en modo topic
 */
bool Channel::topicModeOn() {
    return (((mode & 0x02) >> CH_TOP));
}


/**
 * Comprueba si el canal está en modo moderado
 */
    bool Channel::moderatedModeOn() {
        return (((mode & 0x04) >> CH_MOD));
    }

/**
 * Check if user has operator channel_mode
 */
bool Channel::isUserOperator(User &user) {
    char mask = user.ch_name_mask_map.find(name)->second;
    return (((mask & 0x80) >> 7));
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(std::string &nick) {
    white_list.push_back(nick);
}

/**
 * Devuelve true si el usuario está en la white_list del canal 
 */
bool Channel::isInvited(std::string &nick) {
    return (std::find(white_list.begin(), white_list.end(), nick) != white_list.end());
}

bool Channel::userIsInChannel(string& nick) {
    return (std::find(users.begin(), users.end(), nick) != users.end());
}

void Channel::addMode(int bits) {
    mode |= (0x01 << bits);
}

void Channel::deleteMode(int bits) {
    mode &= ~(0x01 << bits);
}


} // namespace
