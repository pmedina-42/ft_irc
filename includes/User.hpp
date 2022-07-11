#ifndef IRC42_USER_H
# define IRC42_USER_H

#include "Types.hpp"
#include <iostream>

using std::string;

namespace irc {

/* los comandos son muy user dependent. Si alguien envia un comando,
 * lo mejor es que sea desde el objeto que representa ese alguien que
 * se haga lo relacionado con mensaje.
 * Si no, es un follon estar parseando desde una clase que no tiene ni
 * idea de si el usuario existe, de si esta en tal canal, de qué
 * modo tiene, etc etc. Un palazo.
 * 
 * 
 * Todo comando tiene fases:
 * - 1. Asegurarse que el comando es correcto (tiene sintaxis correcta o
 *  aparentemente correcta).
 * - 1.5 Si el comando es correcto pero no está finalizado (enviando ctrl + D con netcat),
 *  esperar a que el comando esté entero (si no timeout?).
 * - 2. Comprobar si el usuario cumple con los requisitos para que el comando
 *  sea ejecutado (está en el canal, el destinatario está también, tiene
 *   los permisos necesarios, etc.).
 * - 3. Ejecutar el comando.
 * 
 * La parte 3 se tiene que necesariamente hacer desde el Servidor, y la parte
 * 2 en muchos casos tambien.
 * Parece ser necesario conectar las clases User y
 * Channel de forma que de uno puedas ir al otro y del otro al uno. Algo como
 * que user tenga acceso a la clase channel de los canales a los que pertenece.
 * Y viceversa, asi, desde user_A podría mirar en #channel_X si user_B está.
 */


class User {

    public:
    User(int fd);
    User(const User &other);
    virtual ~User();

    User& operator=(const User &other);

    void setPrefixFromHost(string &host);
    /* ATTRIBUTES */
    string nick;
    string name;
    string full_name;
    string prefix; // nse si hara falta dis shit
    string mask; // ¿¿¿ XD
    int fd;

    char buffer[SERVER_BUFF_MAX_SIZE];
    int buffer_size;
    bool registered;

    bool hasLeftovers(void) const;
    void resetBuffer(void);
    void addLeftovers(string &leftovers);
    std::string BufferToString(void) const;

    inline string getNick() { return nick; }

    /* PING PONG things
     */
    time_t getLastMsgTime(void);
    time_t getPingTime(void);
    bool isOnPongHold(void);
    void resetPingStatus(void);
    void updatePingStatus(std::string &random);

    bool on_pong_hold;
    time_t last_received;
    time_t ping_send_time;
    string ping_str;

};

}

std::ostream& operator<<(std::ostream &o, irc::User const &rhs);

#endif
