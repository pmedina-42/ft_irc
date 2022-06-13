#include "Command.hpp"
#include <string>
#include <vector>
#include "Tools.hpp"

using std::string;
using std::vector;

namespace irc {

Command::Command(string &recieved)
:
    cmd(recieved)
{}

Command::~Command() 
{}

/* Le llega el comando sin CRLF. Será el servidor el
 * encargado de splitear los buffer en CRLF.
 * Esto es necesario ya que cada commando responderá a un
 * nombre, y cada nombre a una función. Entonces, la mecánica
 * es ir pasando, dado un buffer, todas las líneas separadas
 * por CRLF de una en una, y que el servidor vaya llamando
 * a su CommandMap[command.name], hasta que se acabe el
 * tamaño del buffer o se de un error.
 * De esta forma también evitamos tener que gestionar CRLF a secas
 * dentro de Parse, ya que según el RFC los comandos vacíos 
 * deben ignorarse. 
 */
int Command::Parse() {

    if (tools::newlines_left(cmd)) {
        return ERR_NEWLINES;
    }
    if (tools::colon_placed_incorrectly(cmd)) {
        return ERR_COLONS;
    }

    vector<string> colon_split;
    vector<string> space_split;

    colon_split = tools::split(colon_split, cmd, ":");
    /* If there was a colon split, only split in spaces first arg. */
    if (colon_split.size() == 2) {
        space_split = tools::split(space_split, colon_split[0], " ");
    /* else just split in spaces original command */
    } else {
        space_split = tools::split(space_split, cmd, " ");
    }

    if (tools::is_upper_case(space_split[0]) == false) {
        return ERR_NO_COMMAND;
    }
    args = space_split;
    if (colon_split.size() == 2) {
        args.push_back(colon_split[1]);
    }
    return OK;
}

string Command::Name() {
    if (args.empty() == false) {
        return args[0];
    }
    return "";
}

} // namespace