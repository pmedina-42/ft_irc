#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

#include "Types.hpp"

namespace irc {

class User;
/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
    public:
    Channel(std::string name, User& user);
    ~Channel();

    /* Class functions */
    void addUser(User& user, std::string channel, std::string mode);
    void deleteUser(User& user);
    void banUser(User& user);
    void unbanUser(User& user);
    bool userInBlackList(User& user);
    /* Esta función devuelve true si el canal está en modo invitación */
    bool inviteModeOn();
    bool keyModeOn();
    bool topicModeOn();
    /* Esta función devuelve true si un usuario está invitado al canal
     * si devuelve true, el usuario puede unirse al canal, si no no */
    bool isInvited(User& user);
    void setUserMode(User& user, char mode);
    void addToWhitelist(User& user);
    User& getUserFromFd(int fd);
    User& getUserFromNick(std::string& nick);
    bool userIsInChannel(int userFd);
    bool userIsInChannel(std::string& nick);
    bool isUserOperator(User& user);
    User& findUserByNick(std::string& nick);
    void setUserMode(User &user, std::string mode);

    /* ATTRIBUTES */
    /* Lista de usuarios que pertenecen al canal */
    UserList users;
    /* Lista de usuarios que son operadores */
    UserList _oper_users;
    /* Lista de usuarios invitados al canal */
    NickUserMap white_list; // list maybe  ?
    std::list<std::string> black_list;

    /* Nombre del canal */
    std::string name;
    /* Modo del canal */
    std::string mode;
    /* Maximo de usuarios en el canal */
    unsigned int _max_users;
    /* Clave, si es que la tiene y el modo es k */
    std::string key;
    /**
     * Channel topic
     */
    std::string topic;

};

} // namespace

#endif
