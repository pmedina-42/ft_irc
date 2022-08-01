#ifndef IRC42_USER_H
# define IRC42_USER_H

#include "Types.hpp"
#include <iostream>

namespace irc {

/*
 * Los usuarios tendrán 2 nicks : El que dicen que tienen (real),
 * y el que se guardará en los mapas. Esto es para poder hacer de
 * forma directa la busqueda de usuarios cuando se les envía un 
 * mensaje, sin tener que estar haciendo comparaciones con tolower.
 * Se aprovecha la funcion en tools de ToUpperCase para crear nick 
 * a partir de real_nick (el recibido).
 * El prefijo, de todas formas, seguirá reflejando el real_nick,
 * ya me jodería que si me quiero llamar ChRiStIAn el servidor me
 * llame CHRISTIAN. El control interno tiene que quedar como es, interno.
 *
 */

class User {

    public:
    User(int fd);
    User(const User &other);
    ~User();

    User& operator=(const User &other);
    bool operator==(User const &other) const;

    void setPrefixFromHost(std::string &host);
    /* ATTRIBUTES */
    int fd;
    std::string real_nick; /* caRCe-b042 */
    std::string nick;     /* CARCE-B042 */
    std::string name;
    std::string full_name;
    std::string prefix;
    std::string mask;
    std::string server_mode;
    std::string afk_msg;
    std::string connection_pass;

    /* Channel Things */
    ChannelModeMap channel_mode;
    ChannelMaskMap ch_name_mask_map;
    void addChannelMask(std::string &name, std::string mode);
    void deleteChannelMask(std::string &channel);

    char buffer[BUFF_MAX_SIZE];
    int buffer_size;
    bool registered;

    bool hasLeftovers(void) const;
    void resetBuffer(void);
    void addLeftovers(std::string &leftovers);
    std::string BufferToString(void) const;

    /* PING PONG things */
    time_t getLastMsgTime(void);
    time_t getPingTime(void);
    bool isOnPongHold(void);
    void resetPingStatus(void);
    void updatePingStatus(std::string &random);

    bool isResgistered(void);

    bool on_pong_hold;
    time_t last_received;
    time_t ping_send_time;
    std::string ping_str;

};

}

std::ostream& operator<<(std::ostream &o, const irc::User &rhs);

#endif
