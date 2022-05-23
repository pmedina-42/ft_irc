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
namespace utils {

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

} // utils 
} // irc

#endif /* IRC42_UTILS_H */