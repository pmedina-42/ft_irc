#ifndef IRC42_MESSAGE_H
#define IRC42_MESSAGE_H

#include <string>

using std::string;
/**
 * * Definir una clase con todos los prefijos 
 * * La clase mensaje recibe el mensaje en bruto y desde dentro se parsea
 * 
 * Servidor tiene mapa para canales y usuarios :
 * - Cuando se conecta un usuario, entra en la lista de usuarios.
 * - Cuando un usuario crea un canal, se crea un canal en el mapa
 * y se copia ese usuario dentro del canal.
 * SERVIDOR:
 * -- MAPA DE CANALES CON CLAVE DE NOMBRE DE CANAL, contiene
 * Channel con su whitelist/blacklist, tipo de canal etc. i.e., de tipo
 * Servidor->Mapa_canales := std::map<string=name, Channel>
 * 
 * -- MAPA DE USUARIOS CON CLAVE DE NICKNAME, que contiene un 
 * objeto usuario con sus mierdas. Ejemplos de sus mierdas: fd, 
 * real name. NO se incluyen aquí los canales en los que está, los modos,
 * si está en algun whitelist, etc.
 * 
 * -- Buffer de envío/recepción
 * 
 * *******
 * _Comment______
 * Habrá clase Interfaz de User, de la que el usuario, al estar en un
 * canal, se creará dentro del canal una entrada de tipo ChannelUser.
 * ChannelUser hereda de User y le añade cosas como el modo, si está
 * baneado(black). La whitelist estará a nivel de canal en un std::set
 * (es mejor a la hora de solo guardar un tipo de dato, y de forma 
 * ordenada.)
 * *******
 * 
 * CANAL:
 * -- MAPA DE USUARIOS CON CLAVE DE NICKNAME, que contiene un objeto
 * de tipo usuario PERO del que hereda de usuario, ChannelUser o lo que sea.
 * - modo del canal, 
 * 
 * 
 * Para no preocuparse sobre cómo el mapa aloja o no memoria al añadir
 * elementos :
 * https://stackoverflow.com/questions/22923236/c-stdmap-memory-management
 * Por qué usar unordered_map en vez de map :
 * https://stackoverflow.com/questions/2196995/is-there-any-advantage-of-using-map-over-unordered-map-in-case-of-trivial-keys
 * 
 */
namespace irc {

/* generic message class 
 * 
 * Esto tendrá que tener setters que throween errores
 * en caso de que, por ejemplo, el comando non exista,
 * o el mensaje exceda el número de caracteres máximo,
 * el destino no exista, etc.
 * */
class Message {

    public:
    Message(string& to_parse);
    ~Message(void);

    void parseMessage(void);

    string buffer[512];
    string prefix;
    string dest;
    string src;
};

std::ostream&   operator<<( std::ostream &o, Message const &rhs );

}

#endif /* IRC42_MESSAGE_H */