#include <vector>
#include <string>

using std::string;
using std::vector;

namespace irc {
namespace utils {    

vector<string>& split(vector<string> &to_fill, string &str, char sep);

}
}