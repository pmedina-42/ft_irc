#ifndef IRC42_TOOLS_H
# define IRC42_TOOLS_H

#include <vector>
#include <string>

using std::string;
using std::vector;

namespace irc {
namespace tools {

vector<string>& split(vector<string> &to_fill, string &str, char sep);
vector<string>& split(vector<string> &to_fill, const char* buff,
                      size_t bufflen, string &del);
vector<string>& split(vector<string> &to_fill, string &str, string del);

void to_upper_case(string &str);
bool is_equal(const string &str1, const string &str2);
bool ends_with(std::string const &str, std::string const &suffix);

string& trim_repeated_char(string& str, char c);

void clean_buffer(char *buff, size_t size);
void printError(string error_str);

string rng_string(int len);

} // tools
} // irc

#endif /* IRC42_TOOLS_H */