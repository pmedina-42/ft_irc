#include "Tools.hpp"
#include "Exceptions.hpp"
#include "Types.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

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
        string aux(c_matrix[i], strlen(c_matrix[i]));
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
    string str(buff, bufflen);
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
    std::string last = str.substr(start, end - start);
    if (!last.empty()) {
        to_fill.push_back(str.substr(start, end - start));
    }
    return to_fill;
}

string& trim_repeated_char(string& str, char c) {
    string clean_str;
    size_t src_size = str.size();
    clean_str.reserve(src_size); // max possible
    
    for (size_t i = 0; i < src_size; i++) {
        clean_str.push_back(str[i]);
        if (str[i] == c) {
            while (str[i] == c) {
                i++;
            }
            i--; // counter increment
        }
    }
    str = clean_str;
    return str;
}

/*
 * Each lowercase letter is 32 + uppercase equivalent. This means simply
 * flipping the bit at position 5 (counting from least significant bit
 * at position 0) inverts the case of a letter.
 */
void to_upper_case(string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it >= 'a' && *it <= 'z') {
            *it ^= 0x20;
        }
    }
}

/* checks if a string is equal to another, ignoring case
 * differences (HellO = hELlo) */
bool is_equal(const string &str1, const string &str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    size_t len = str1.length();
    for (size_t i = 0; i < len; i++) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

/* Check if str ends with suffix */
bool ends_with(std::string const &str, std::string const &suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return (str.compare(str.length() - suffix.length(),
                        suffix.length(), suffix) == 0);
}

/* 
 * from https://stackoverflow.com/questions/440133
 * See https://cplusplus.com/reference/cstdlib/rand/
 */
string rng_string(int len) {
    srand((unsigned)time(NULL) * getpid());
    static const unsigned char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!$%&/()=?^*_;:\\|@#~[]{},-'+";
    std::string tmp_s;
    tmp_s.reserve(len);
    for (int i = 0; i < len; ++i) {
        unsigned char c = alphanum[rand() % (sizeof(alphanum))];
        tmp_s.push_back(c);
    }
    return tmp_s;
}

/* 
 * returns the index corresponding to the last crlf, e.g. :
 * from "Hello CRLF lol", returns the position 5.
 * This is done to then call substr(0, pos),
 * which will be the contents of the string before
 * the last CRLF.
 */
size_t find_last_CRLF(string& haystack) {
    
    size_t CR_pos = haystack.find_last_of(CR);
    size_t LF_pos = haystack.find_last_of(LF);

    if (CR_pos != string::npos
        && LF_pos != string::npos)
    {
        if (LF_pos == CR_pos + 1) {
            return CR_pos - 1;
        }
    }
    return string::npos;
}


void clean_buffer(char *buff, size_t size) {
    if (size > 0 && buff != NULL) {
        memset(buff, '\0', size);
    }
}

void printError(string error_str) {
    std::cerr << error_str << std::endl;
}

} // tools 
} // irc
