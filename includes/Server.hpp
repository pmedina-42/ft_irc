#ifndef IRC42_SERVER_H
# define IRC42_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <poll.h>
#include "Channel.hpp" // without this, it doesnt compile
#include "Types.hpp"

using std::string;

namespace irc {

class FdManager {
    public:
    FdManager(void);
    FdManager(FdManager &rhs);
    ~FdManager(void);

    void setUpListener(void);
    bool hasDataToRead(int entry);
    bool skipFd(int fd_idx);
    int AcceptConnection(void);
    void CloseConnection(int fd_idx);
    void Poll(void);

    int getSocketError(int);
    /* fd from clients manager. This includes
    * the listener, at entry 0.
    */
    struct pollfd fds[MAX_FDS];
    int fds_size;
    /* addresses corresponding to each client */
    struct sockaddr *addr[MAX_FDS];

    struct addrinfo *servinfo;
    int listener;
};

class Server {

    public:
    Server(void);
    Server(string &ip, string &port);
    Server(Server &rhs);
    ~Server();
    
    private:
    /* Setup */
    int setServerInfo(void);
    int setServerInfo(string &hostname, string &port);
    int setListener(void);
    void loadCommandMap(void);

    /* parses message into commands, calls 
     * commands from user until finished. */
    void DataFromUser(int fd_idx);
    void DataToUser(int fd_idx, string &data);

    /* loop through all poll fd's */
    int mainLoop(void);
    void AddNewUser(int new_fd);
    void RemoveUser(int fd_idx);
    string processCommandBuffer(int fd_idx);

    char srv_buff[SERVER_BUFF_MAX_SIZE];
    int srv_buff_size;
    string hostname;
    
    ChannelMap channel_map; /* Find channels by name */
    NickFdMap nick_fd_map;  /* Find users by nickname */
    FdUserMap fd_user_map; /* Find nickname by fd_idx (entry of fds[]) */

    /* Socket related stuff */
    FdManager    fd_manager;

    /* Map with all command responses */
    CommandMap cmd_map;

    /* command implementations */
    void NICK(Command &cmd, int fd);
    void USER(Command &cmd, int fd);
    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);

    bool nickAlreadyInUse(string &nickname);
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
