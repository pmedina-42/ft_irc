#ifndef CHANNEL_H
#define CHANNEL_H

#include <list>
#include <string>

using std::list;
using std::string;

namespace irc {

class User;
/* TODO: comprobar y setear tamaño máximo de usuarios dentro de un canal? */

class Channel {
    public:
    Channel(string,  User*);
    ~Channel();

    /* Class functions */
    void addUser(User*);
    void deleteUser(string);
    void banUser(string);
    void unbanUser(string);
    bool userInBlackList(string);
    /* Esta función devuelve true si el canal está en modo invitación */
    bool inviteModeOn();
    /* Esta función devuelve true si un usuario está invitado al canal
        * si devuelve true, el usuario puede unirse al canal, si no no */
    bool isInvited(User*);
    void setUserMode(string, char);
    void addToWhitelist(User*);

    /* ATTRIBUTES */
    /* Lista de usuarios que pertenecen al canal */
    list<User*> _users;
    /* Lista de usuarios que son operadores */
    list<User*> _oper_users;
    /* Lista de usuarios invitados al canal */
    list<User*> _whiteList;
    /* Lista de nombres de usuarios baneados */
    list<string> _blackList;

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

};

} // namespace

#endif
