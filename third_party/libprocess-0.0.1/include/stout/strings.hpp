#ifndef __STOUT_STRINGS_HPP__
#define __STOUT_STRINGS_HPP__

#include <stdarg.h>
#include <stdio.h>

#include <string>
#include <map>
#include <vector>

namespace strings {

inline std::string format(const std::string& s, va_list args)
{
  char* temp;
  if (vasprintf(&temp, s.c_str(), args) == -1) {
    // Note that temp is undefined, so we do not need to call free.
    std::cerr << "Failed to format '" << s
              << "' (possibly out of memory)" << std::endl;
    abort();
  }
  std::string result(temp);
  free(temp);
  return result;
}


// TODO(benh): Create a version of format which uses templates to
// check if an argument is a std::string and if so calls the var args
// version using std::string::c_str.
inline std::string format(const std::string& s, ...)
{
  va_list args;
  va_start(args, s);
  const std::string& result = format(s, args);
  va_end(args);
  return result;
}


// Flags indicating how remove should operate.
enum Mode {
  PREFIX,
  SUFFIX,
  ANY
};


inline std::string remove(
    const std::string& from,
    const std::string& substring,
    Mode mode = ANY)
{
  std::string result = from;

  if (mode == PREFIX) {
    if (from.find(substring) == 0) {
      result = from.substr(substring.size());
    }
  } else if (mode == SUFFIX) {
    if (from.rfind(substring) == from.size() - substring.size()) {
      result = from.substr(0, from.size() - substring.size());
    }
  } else {
    size_t index;
    while ((index = result.find(substring)) != std::string::npos) {
      result = result.erase(index, substring.size());
    }
  }

  return result;
}


inline std::string trim(
    const std::string& from,
    const std::string& chars = " \t\n\r")
{
  size_t start = from.find_first_not_of(chars);
  size_t end = from.find_last_not_of(chars);
  if (start == std::string::npos) { // Contains only characters in chars.
    return "";
  }

  return from.substr(start, end + 1 - start);
}


inline std::vector<std::string> split(
    const std::string& s,
    const std::string& delims)
{
  std::vector<std::string> tokens;

  size_t offset = 0;
  while (true) {
    size_t i = s.find_first_not_of(delims, offset);
    if (std::string::npos == i) {
      offset = s.length();
      return tokens;
    }

    size_t j = s.find_first_of(delims, i);
    if (std::string::npos == j) {
      tokens.push_back(s.substr(i));
      offset = s.length();
      continue;
    }

    tokens.push_back(s.substr(i, j - i));
    offset = j;
  }
}


inline std::map<std::string, std::vector<std::string> > pairs(
    const std::string& s, char delim1, char delim2)
{
  std::map<std::string, std::vector<std::string> > result;

  const std::vector<std::string>& tokens = split(s, std::string(1, delim1));
  std::vector<std::string>::const_iterator = tokens.begin();
  while (iterator != tokens.end()) {
    const std::string& token = *iterator;
    const std::vector<std::string>& pairs =
      split(token, std::string(1, delim2));
    if (pairs.size() == 2) {
      result[pairs[0]].push_back(pairs[1]);
    }
    ++iterator;
  }

  return result;
}

} // namespaces strings {

#endif // __STOUT_STRINGS_HPP__
