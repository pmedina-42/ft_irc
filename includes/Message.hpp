#ifndef IRC42_MESSAGE_H
#define IRC42_MESSAGE_H

#include <string>

using std::string;

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