#include "Command.hpp"
#include <string>
#include <vector>
#include "Tools.hpp"
#include <iostream>


using std::string;
using std::vector;

namespace irc {

Command::Command(void)
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
int Command::Parse(string &cmd) {

    if (tools::newlines_left(cmd)) {
        return ERR_NEWLINES;
    }
    if (tools::colon_placed_incorrectly(cmd)) {
        return ERR_COLONS;
    }
    cmd = tools::trim_repeated_char(cmd, ' ');
    
    if (cmd[0] == ':') {
        int prefix_end = cmd.find(" ");
        // eliminate prefix in case specified
        if (prefix_end != -1) {
            cmd = cmd.substr(prefix_end);
        }
    }
    vector<string> colon_split;
    vector<string> space_split;

    colon_split = tools::split(colon_split, cmd, ":");
    /* If there was a colon split, only split in spaces first arg. */
    if (colon_split.size() == 2) {
        space_split = tools::split(space_split, colon_split[0], " ");
        colon_split[1].insert(0, ":");
    /* else just split in spaces original command */
    } else {
        space_split = tools::split(space_split, cmd, " ");
    }

    for (int i = 0; i < (int)space_split.size(); i++) {
        std::cout << "sp_split : [" << space_split[i] << "]"<< std::endl;
    }

    if (tools::is_upper_case(space_split[0]) == false) {
        return ERR_NO_COMMAND;
    }
    args = space_split;
    if (colon_split.size() == 2) {
        args.push_back(colon_split[1]);
    }
    debugCommand();
    return OK;
}

string Command::Name() {
    if (args.empty() == false) {
        return args[0];
    }
    return "";
}

void Command::debugCommand() const {
    std::cout << std::endl << "COMMAND RESULT : " << std::endl;
    size_t size = args.size();
    std::cout << "Command vector size : " << args.size();
    std::cout << ", content :" << std::endl;
    for (size_t it = 0; it < size; it++) {
        std::cout << it << ": [" << args[it] << "]" << std::endl;
    }
    std::cout << std::endl;
}


} // namespace