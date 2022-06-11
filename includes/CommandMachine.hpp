#ifndef IRC42_COMMAND_MACHINE_H
# define IRC42_COMMAND_MACHINE_H

#include <list>
#include <string>
#include <vector>
#include "Types.hpp"

using std::string;
using std::vector;

namespace irc {

/* Managea cada comando que llegue. Está dedicado a resolver
 * un comando en exclusive, i.e. [CMDNAME <args ... > : ... ]
 */
class Command {

    public:
    Command();
    ~Command();

    int isValidCommand(string &recieved);

    /* Esto tendrá que tener en su momento, una especie de mapa s.t.:
     * [USER] -> [function que managea user]
     * [NICK] -> [funcion que managea nick]
     * .
     * .
     * .
     */
    string cmd;
    string from;
    string cmd_name; // USER 
    vector<string> args;

    private:
};

/* Dirige a la lista de comandos. quizás no acabe haciendo falta.
 * De momento se dedica a dividir cada lectura del socket, y crea
 * tantos comandos como se reciban.
 */
class CommandMachine {
    
    public:
    CommandMachine(const char* buff, size_t len);
    ~CommandMachine();

    enum BUFFER_STATE {
        CLEAR = 0,
        DATA_READY,
        PENDING_DATA,
    };

    int parseBuffer(char* , );

    std::string buffer;
    CommandList cmd_lst;
    int state;
};

} // namespace

#endif /*IRC42_COMMAND_MACHINE_H*/