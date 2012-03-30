#ifndef __STOUT_SH_HPP__
#define __STOUT_SH_HPP__

#include <stdarg.h>
#include <stdio.h>

#include <stout/strings.hpp>
#include <stout/try.hpp>

// Runs a shell command formatted with varargs and return the return value
// of the command. Optionally, the output is returned via an argument.
inline Try<int> sh(std::iostream* ios, const std::string& format, ...)
{
  va_list args;
  va_start(args, format);

  const Try<std::string>& command = strings::format(format, args);

  va_end(args);

  if (command.isError()) {
    return Try<int>::error(command.error());
  }

  FILE* file;

  if ((file = ::popen(command.get().c_str(), "r")) == NULL) {
    return Try<int>::error("Failed to run '" + command.get() + "'");
  }

  char line[1024];
  // NOTE(vinod): Ideally the if and while loops should be interchanged. But
  // we get a broken pipe error if we don't read the output and simply close.
  while (::fgets(line, sizeof(line), file) != NULL) {
    if (ios != NULL) {
      *ios << line ;
    }
  }

  if (::ferror(file) != 0) {
    const std::string& error = strings::format(
        "Error reading output of '%s': %s",
        command.get().c_str(), strerror(errno));
    ::pclose(file); // Ignoring result since we already have an error.
    return Try<int>::error(error);
  }

  int status;
  if ((status = ::pclose(file)) == -1) {
    return Try<int>::error(
        "Failed to get exit/termination status of '" + command.get() + "'");
  }

  return status;
}

#endif // __STOUT_SH_HPP__
