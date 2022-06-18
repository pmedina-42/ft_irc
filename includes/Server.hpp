#ifndef IRC42_SERVER_H
#define IRC42_SERVER_H

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <poll.h>
#include <vector>
#include "Types.hpp"

#define LISTENER_BACKLOG 20
#define NAME_MAX_SZ 10
#define MAX_FDS 255

#define SERVER_BUFF_MAX_SIZE 512

using std::string;
using std::vector;

class User;
class Channel;

namespace irc {

class AddressInfo {
    public:
    AddressInfo(void);
    AddressInfo(AddressInfo &rhs);
    ~AddressInfo(void);

    /* ptr to struct addrinfo list */
    struct addrinfo *servinfo;
    /* actual list entry being used */
    struct addrinfo *actual;
    /* fd set to listen */
    int listener;
};

class FdManager {
    public:
    FdManager(void);
    FdManager(FdManager &rhs);
    ~FdManager(void);

    void setUpListener(int listener);
    bool hasDataToRead(int entry);
    bool hasHangUp(int entry);
    int AcceptConnection(void);
    void CloseConnection(int fd_idx);
    /* fd from clients manager. This includes
    * the listener, at entry 0.
    */
    struct pollfd fds[MAX_FDS];
    int fds_size;
    /* addresses corresponding to each client */
    struct sockaddr *addr[MAX_FDS];
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
    void DataFromUser(int fd, int fd_idx);
    int DataToUser(int fd_idx, string &data);

    /* loop through all poll fd's */
    int mainLoop(void);
    void AddNewUser(int fd);
    void RemoveUser(int fd);

    /* Esto no es definitivo ni mucho menos. Seguramente
     * cada comando acabe gestionandose a si mismo,
     * en vez de trabajar con un return generalizado. */
    typedef enum COMMAND_RESULT {
        OK = 0, // nada que hacer
        ERR_FATAL
    } COMMAND_RESULT;
    
    char srv_buff[513];
    int srv_buff_size;
    string hostname;
    
    ChannelMap channel_map; /* Find channels by name */
    NickFdMap nick_fd_map;  /* Find users by nickname */
    FdUserMap fd_user_map; /* Find nickname by fd_idx (entry of fds[]) */

    /* Socket related stuff */
    AddressInfo _info;
    FdManager    _fd_manager;

    /* Map with all comand responses */
    CommandMap cmd_map;

    /* command implementations */
    int NICK(Command &cmd, int fd);
    int USER(Command &cmd, int fd);

    bool nickAlreadyInUse(string &nickname);
};

/**
 * Weechat handshake init Message : [NICK carce
 *   USER carce 0 * :carce
 *  ]
 * 
 * Funcionamiento interno de las estructuras del servidor:
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
