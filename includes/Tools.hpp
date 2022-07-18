#ifndef IRC42_TOOLS_H
# define IRC42_TOOLS_H

#include <vector>
#include <string>

using std::string;
using std::vector;

namespace irc {
namespace tools {

std::vector<std::string>& split(std::vector<std::string> &to_fill,
                                std::string &str, char sep);
std::vector<std::string>& split(std::vector<std::string> &to_fill,
                                const char* buff, size_t bufflen,
                                std::string &del);
std::vector<std::string>& split(std::vector<std::string> &to_fill,
                                std::string &str, std::string del);

bool starts_with_mask(std::string const);
void ToUpperCase(std::string &str);
bool isEqual(const std::string &str1, const std::string &str2);
bool endsWith(std::string const &str, std::string const &suffix);

std::string& trimRepeatedChar(std::string& str, char c);
void ReplaceAll(std::string& str, const std::string& from,
                                  const std::string& to);
size_t findLastCRLF(std::string& haystack);

void cleanBuffer(char *buff, size_t size);
void printError(std::string error_str);

std::string rngString(int len);

} // tools
} // irc

#endif /* IRC42_TOOLS_H */
