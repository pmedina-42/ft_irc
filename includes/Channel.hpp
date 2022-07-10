#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

#include "Types.hpp"

using std::list;
using std::map;
using std::string;

namespace irc {

class ChannelUser;
/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
    public:
    Channel(string name, ChannelUser&);
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
    ChannelUser userInChannel(Channel&, int fd);
    static bool isUserOperator(ChannelUser&);

    /* ATTRIBUTES */
    /* Lista de usuarios que pertenecen al canal */
    ChannelUserList users;
    /* Lista de usuarios que son operadores */
    ChannelUserList _oper_users;
    /* Lista de usuarios invitados al canal */
    ChannelUserMap whiteList; // list maybe  ?

    /* Nombre del canal */
    string _name;
    /* Tema del canal */
    string _topic;
    /* Modo del canal */
    char mode;
    /* Maximo de usuarios en el canal */
    unsigned int _max_users;
    /* Clave, si es que la tiene y el modo es k */
    string _key;
    /**
     * Channel topic
     */
    string topic;

};

} // namespace

#endif
