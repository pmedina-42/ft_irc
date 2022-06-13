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

bool is_upper_case(std::string &str);
bool colon_placed_incorrectly(std::string &str);
bool newlines_left(std::string &str);

} // tools
} // irc

#endif /* IRC42_TOOLS_H */