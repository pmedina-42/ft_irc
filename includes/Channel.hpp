#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

#include "Types.hpp"
#include "ChannelUser.hpp"

using std::list;
using std::map;
using std::string;

namespace irc {

class ChannelUser;
/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
    public:
    Channel(char prefix, string name, ChannelUser&);
    ~Channel();

    /* Class functions */
    void addUser(ChannelUser&);
    void deleteUser(ChannelUser&);
    void banUser(ChannelUser&);
    void unbanUser(ChannelUser&);
    bool userInBlackList(ChannelUser&);
    /* Esta función devuelve true si el canal está en modo invitación */
    bool inviteModeOn();
    /* Esta función devuelve true si un usuario está invitado al canal
        * si devuelve true, el usuario puede unirse al canal, si no no */
    bool isInvited(ChannelUser&);
    void setUserMode(ChannelUser&, char);
    void addToWhitelist(ChannelUser&);

    /* ATTRIBUTES */
    /* Lista de usuarios que pertenecen al canal */
    ChannelUserMap users;
    /* Lista de usuarios que son operadores */
    list<ChannelUser*> _oper_users;
    /* Lista de usuarios invitados al canal */
    map<string, ChannelUser> whiteList; // list maybe  ?

    /* Nombre del canal */
    string _name;
    /* Tema del canal */
    string _topic;
    /* Modo del canal */
    char _mode;
    /* Maximo de usuarios en el canal */
    unsigned int _max_users;
    /* Clave, si es que la tiene y el modo es k */
    string _key;
    /* El prefio que va a tener el canal #, &, + o ! */
    char _prefix;

    /**
     * Si tenemos un mapa de usuarios necesitamos 2 cosas:
     * 1. Un int que nos indique el tamaño actual del canal
     * 2. Otro int que nos indique el índice de usuario por el que vamos:
     *      Si se ha ido el usuario de pos 1, tenemos que encontrar el usuario de pos 2
     *      (y comprobar que no esta baneado) para darle a el el rol de operador.
     *      Si se va el 2 ahora necesitaremos darle el rol de operador a index + 1
     */
     int actualSize;
     int userIndex;

};

} // namespace

#endif
