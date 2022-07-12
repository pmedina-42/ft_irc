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

bool starts_with_mask(string const);
void ToUpperCase(string &str);
bool isEqual(const string &str1, const string &str2);
bool endsWith(string const &str, string const &suffix);

string& trimRepeatedChar(string& str, char c);
void ReplaceAll(string& str, const string& from, const string& to);
size_t findLastCRLF(string& haystack);

void cleanBuffer(char *buff, size_t size);
void printError(string error_str);

string rngString(int len);

} // tools
} // irc

#endif /* IRC42_TOOLS_H */
