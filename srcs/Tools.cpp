#include "Tools.hpp"
#include "Exceptions.hpp"

#include <vector>
#include <string>
#include <iostream>

#include "libft.h"

extern "C" {
#include "string.h"
}

using std::string;
using std::vector;

namespace irc {
namespace tools {

/* cpp adaptation to split C function (using native calls)
 *
 * Note : call with sep = ':', then sep = ' ' and there you go
 * parsed string.
 */
vector<string>& split(vector<string> &to_fill, string &str, char sep) {

    char **c_matrix = NULL;

    c_matrix = ft_split(str.c_str(), sep);
    if (c_matrix == NULL) {
        throw irc::exc::MallocError();
    }
    for (int i = 0; c_matrix[i] != NULL; i++) {
        std::string aux(c_matrix[i], strlen(c_matrix[i]));
        to_fill.push_back(aux);
        /* once written to the vector, free */
        free(c_matrix[i]);
    }
    free(c_matrix);
    return to_fill;
}

/* More potent version. Does not use dynamic allocation explicitly,
 * and splits by string delimiters. It also parses bytearrays instead
 * of null terminated strings.
 */
vector<string>& split(vector<string> &to_fill, const char* buff,
                      size_t bufflen, string &del)
{
    std::string str(buff, bufflen);
    int start = 0;
    int end = str.find(del);
    while (end != -1) {
        to_fill.push_back(str.substr(start, end - start));
        start = end + del.size();
        end = str.find(del, start);
    }
    to_fill.push_back(str.substr(start, end - start));
    return to_fill;
}

vector<string>& split(vector<string> &to_fill, string &str, string del) {
    int start = 0;
    int end = str.find(del);
    while (end != -1) {
        to_fill.push_back(str.substr(start, end - start));
        start = end + del.size();
        end = str.find(del, start);
    }
    to_fill.push_back(str.substr(start, end - start));
    return to_fill;
}

bool is_upper_case(std::string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it < 'A' && *it > 'Z')
            return false;
    }
    return true;
}

/* Checks that the buffer recieved, in case it has a colon, 
 * it is places separating two pieces of text.
 * (1) msg = * : * OK 
 * (2) msg = :* * :* OK
 * (3) msg = :* * OK
 * (4) msg = *
 * anything else throws an error. Second example corresponds
 * to some clients that send :user_info as a prefix to all
 * messages (must be ignored, but still).
 */
bool colon_placed_incorrectly(string &str) {
    vector<string> result;
    result = split(result, str, string(":"));
    size_t size = result.size();
    if (size < 1 || size > 2) {
        return true;
    }
    if (result[0].empty()) {
        return true;
    }
    /* si no viene despues de un espacio, tampoco vale. Basta con
     * ver si la penúltima string acaba en espacio. Esto
     * es funcdamental de cara a hacer primero un split de ':' y
     * luego uno de ' ' */
    string before_end = result[result.size() - 2];
    if (before_end[before_end.size() - 1] != ' ') {
        return true;
    }
    return false;
}

bool newlines_left(string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it == '\n' && *it == '\r')
            return true;
    }
    return false;
}

void clean_buffer(char *buff, size_t size) {
    if (size > 0 && buff != NULL)
    memset(buff, '\0', size);
}

void printError(string error_str) {
    std::cerr << error_str << std::endl;
}

} // tools 
} // irc