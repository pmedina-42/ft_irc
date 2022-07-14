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
    bool keyModeOn();
    /* Esta función devuelve true si un usuario está invitado al canal
     * si devuelve true, el usuario puede unirse al canal, si no no */
    bool isInvited(ChannelUser&);
    void setUserMode(ChannelUser&, char);
    void addToWhitelist(ChannelUser&);
    ChannelUser& userInChannel(int fd);
    bool isUserOperator(ChannelUser&);
    ChannelUser& findUserByNick(string nick);

    /* ATTRIBUTES */
    /* Lista de usuarios que pertenecen al canal */
    ChannelUserList users;
    /* Lista de usuarios que son operadores */
    ChannelUserList _oper_users;
    /* Lista de usuarios invitados al canal */
    NickChannelUserMap white_list; // list maybe  ?

    /* Nombre del canal */
    string name;
    /* Tema del canal */
    string _topic;
    /* Modo del canal */
    string mode;
    /* Maximo de usuarios en el canal */
    unsigned int _max_users;
    /* Clave, si es que la tiene y el modo es k */
    string key;
    /**
     * Channel topic
     */
    string topic;

};

} // namespace

#endif
