#include <algorithm>
#include <string>
#include <vector>

namespace aspirin::string {

// Return a lower-case version of the given string (warning: not unicode compliant)
inline std::string to_lower(const std::string &s) {
  std::string result;
  result.resize(s.length());
  std::transform(s.begin(), s.end(), result.begin(), ::tolower);
  return result;
}

// Return a upper-case version of the given string (warning: not unicode compliant)
inline std::string to_upper(const std::string &s) {
  std::string result;
  result.resize(s.length());
  std::transform(s.begin(), s.end(), result.begin(), ::toupper);
  return result;
}

inline bool starts_with(const std::string &string, const std::string &prefix) {
  if (prefix.size() > string.size())
    return false;
  return std::equal(prefix.begin(), prefix.end(), string.begin());
}

inline bool ends_with(const std::string &string, const std::string &suffix) {
  if (suffix.size() > string.size())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), string.rbegin());
}

extern std::vector<std::string> tokenize(const std::string &string,
                                         const std::string &delim = ", ",
                                         bool include_empty = false);

extern std::string indent(const std::string &string, size_t amount = 2);

inline void replace_inplace(std::string &str, const std::string &source,
                            const std::string &target) {
  size_t pos = 0;
  while ((pos = str.find(source, pos)) != std::string::npos) {
    str.replace(pos, source.length(), target);
    pos += target.length();
  }
}

}  // namespace misaki::string