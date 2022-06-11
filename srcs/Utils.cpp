#ifndef IRC42_UTILS_H
#define IRC42_UTILS_H

#include "Utils.hpp"
#include "Exceptions.hpp"

#include <vector>
#include <string>

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

bool is_upper_case(std::string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it < 'A' && *it > 'Z')
            return false;
    }
    return true;
}

} // tools 
} // irc

#endif /* IRC42_UTILS_H */