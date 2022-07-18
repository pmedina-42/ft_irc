#ifndef IRC42_SERVER_H
# define IRC42_SERVER_H

#include "Channel.hpp" // without this, it doesnt compile
#include "Types.hpp"

#include "Server/FdManager.hpp"
#include "Server/IrcDataBase.hpp"

namespace irc {

class Server : public FdManager,
               public IrcDataBase
{
    public:
    Server(void);
    Server(std::string &ip, std::string &port);
    Server(const Server &other);
    ~Server();
    
    private:
    /* loop through all poll fd's */
    int mainLoop(void);
    void DataFromUser(int fd);
    void DataToUser(int fd, std::string &data, int type);

    char srv_buff[BUFF_MAX_SIZE];
    int srv_buff_size;
    std::string processLeftovers(int fd); // leftovers
    void parseCommandBuffer(std::string &cmd_content, int fd);

    /* ping pong flow */
    void pingLoop(void);
    void sendPingToUser(int fd);
    
    CommandMap cmd_map;
    void loadCommandMap(void);
    void NICK(Command &cmd, int fd);
    void USER(Command &cmd, int fd);
    void PING(Command &cmd, int fd);
    void PONG(Command &cmd, int fd);
    void MODE(Command &cmd, int fd);
    void PASS(Command &cmd, int fd);
    void AWAY(Command &cmd, int fd);
    void QUIT(Command &cmd, int fd);
    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);
    void TOPIC(Command &cmd, int fd);
    void INVITE(Command &cmd, int fd);

    /* Common replies */
    void sendWelcome(std::string& name, std::string &prefix, int fd);
    void sendNeedMoreParams(std::string& cmd_name, int fd);
    void sendNotRegistered(std::string &cmd_name, int fd);
    void sendNoSuchChannel(std::string &cmd_name, int fd);
    void sendNotOnChannel(std::string &cmd_name, int fd);
    void sendBadChannelMask(std::string &cmd_name, int fd);
    void sendNoChannelModes(std::string &cmd_name, int fd);
    void sendChannelOperatorNeeded(std::string &cmd_name, int fd);
    void sendAlreadyRegistered(std::string &nick, int fd);
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
