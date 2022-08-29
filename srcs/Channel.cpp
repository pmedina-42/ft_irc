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
    all_banned = false;
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

/**
 * Añadir un usuario a un canal:
 * 1. Se comprueba la disponibilidad del canal a nivel de comando, antes de llamar esta funcion
 * 2. Se añade el usuario a la lista de usuarios
 */
void Channel::addUser(User &user) {
    users.push_back(user.nick);
}

/*
 * el nick dentro del canal cuando se lo actualicen
 *
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
void Channel::banUser(string &user, int fd) {
    if (!user.compare("*!*@*")) {
        all_banned = true;
    }
    black_list.insert(std::pair<string, int>(user, fd));
}

/**
 * Desbanea a un usuario
 */
bool Channel::unbanUser(string &user) {
    if (black_list.count(user)) {
        std::map<string, int>::iterator it = black_list.find(user);
        if (!user.compare("*!*@*")) {
            all_banned = false;
        }
        black_list.erase(it->first);
        if (black_list.size() == 0) {
            deleteMode(CH_BAN);
        }
        return true;
    }
    return false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(string nick) {
    for (std::map<string, int>::iterator it = black_list.begin(); it != black_list.end(); it++) {
        if (it->first.find(nick) != string::npos) {
            return true;
        }
    }
    return false;
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

string Channel::getModeStr() {
    string mode = keyModeOn() ? "k" : "";
    mode += inviteModeOn() ? "i" : "";
    mode += moderatedModeOn() ? "m" : "";
    mode += banModeOn() ? "b" : "";
    return mode;
}

void Channel::updateUserNick(string &old_nick, string &new_nick) {
    list<string>::iterator it = std::find(users.begin(), users.end(), old_nick);
    if (it != users.end()) {
        *it = new_nick;
    }
}


} // namespace
