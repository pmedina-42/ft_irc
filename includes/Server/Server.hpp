#ifndef IRC42_SERVER_H
# define IRC42_SERVER_H

#include "Channel.hpp" // without this, it doesnt compile
#include "Types.hpp"

#include "Server/FdManager.hpp"

using std::string;

namespace irc {


class Server
:
    public FdManager
{
    public:
    Server(void);
    Server(string &ip, string &port);
    Server(const Server &other);
    ~Server();
    
    private:
    /* loop through all poll fd's */
    int mainLoop(void);
    void DataFromUser(int fd_idx);
    void DataToUser(int fd_idx, string &data, int type);

    void AddNewUser(int new_fd);
    void RemoveUser(int fd_idx);
    string processCommandBuffer(int fd_idx);

    char srv_buff[BUFF_MAX_SIZE];
    int srv_buff_size;

    ChannelMap channel_map; /* Find channels by name */
    NickFdMap nick_fd_map;  /* Find users by nickname */
    FdUserMap fd_user_map; /* Find nickname by fd_idx (entry of fds[]) */

    /* time stuff */
    time_t start;
    void pingLoop(void);
    void sendPingToUser(int fd_idx);
    
    CommandMap cmd_map;
    void loadCommandMap(void);
    /* Map with all command responses */
    void NICK(Command &cmd, int fd_idx);
    void USER(Command &cmd, int fd_idx);
    void PING(Command &cmd, int fd_idx);
    void PONG(Command &cmd, int fd_idx);
    void MODE(Command &cmd, int fd);
    void PASS(Command &cmd, int fd);
    void AWAY(Command &cmd, int fd);
    void QUIT(Command &cmd, int fd);

    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);
    void TOPIC(Command &cmd, int fd);
    void INVITE(Command &cmd, int fd);


    /* command implementations */
    void sendWelcome(string& name, string &prefix, int fd_idx);
    void sendNeedMoreParams(string& cmd_name, int fd_idx);
    void sendNotRegistered(string &cmd_name, int fd_idx);
    void sendNoSuchChannel(string &cmd_name, int fd_idx);
    void sendNotOnChannel(string &cmd_name, int fd_idx);
    void sendBadChannelMask(string &cmd_name, int fd_idx);
    void sendNoChannelModes(string &cmd_name, int fd_idx);
    void sendChannelOperatorNeeded(string &cmd_name, int fd_idx);
    void sendAlreadyRegistered(int fd_idx);

    /* utils */
    User& getUserFromFd(int fd);
    User& getUserFromFdIndex(int fd_idx);
    User& getUserFromNick(string& nickname);
    int getFdFromNick(string& nickname);
    Channel& getChannelFromName(string name);
};

/**
 * Reglas propias servidor :
 * - El número mázimo de usuarios conectados a la vez será de 255 (MAX_FDS).
 * - Los usuarios se guardan, con comandos no finalizados en CRLF, un buffer
 *  interno, de donde reconstruir un comando que se haya enviado troceado.
 *  La suma de los bytes en este buffer y los del comando al que se añaden
 *  nunca podrá ser superior a 512.
 * e.g. Si tengo 200 bytes guardados de USer A (un mensaje muy largo), y 
 * me llega el final de éste ( <loquesea> CRLF), insertaré esos 200 bytes
 * al pricnipio de <loquesea>, y si 200 + len(<loquesea> CRLF) > 512, este
 * comando no será ejecutado y se le enviará al usuario un error de input
 * too long.
 * De forma silenciosa, cuando el buffer interno (que será de 512 bytes) 
 * se llene, cuando llegue un nuevo mensaje que añadir a lo que ya hay 
 * se enviará tambien error de input too long.
 * 
 * 
 * Funcionamiento interno de las estructuras del servidor:
 * 
 * Cuando un usuario se conecta, de él solo se conoce el
 * fd asociado, o la ENTRADA que ocupa en la matriz de fds
 * que usa poll. Ni nickname, ni username, ni nada.
 * Por este motivo, existe un FdUserMap que en función de su
 * índice de fd que ocupe en la matriz de poll, se corresponde
 * con una entrada de User. A su vez, existirá, para amenizar 
 * el envío de mensajes privados o mensajes a canal o peticiones
 * con necesidad de lookup de nombre de usuario, un mapa de
 * clave nickname y valor fd. De esta forma, cuando se tenga un nickname
 * en vez de un fd, se usará el mapa para encontrar el fd correpsondiente,
 * y luego se buscará al usuario dentro del FdUserMap.
 * 
 * 
 * 
 * 
 * Para gestionar mensajes de NICK / USER iniciales en
 * redes con banda ancha muy mala, o troleos con netcat más bien
 * ya que no existe a día de hoy una red que no pueda procesar por
 * tcp en un solo paquete 30 bytes, es necesario poder reconocer
 * a un usuario por su fd antes incluso de que nos haya proporcionado
 * un nickname. Y cuando s
 */
}

#endif /* IRC42_SERVER_H */
