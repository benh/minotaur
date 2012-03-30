#ifndef __STOUT_OS_HPP__
#define __STOUT_OS_HPP__

#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#ifdef __linux__
#include <sys/sysinfo.h>
#endif
#include <sys/types.h>

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <stout/strings.hpp>
#include <stout/try.hpp>

#ifdef __APPLE__
#define gethostbyname2_r(name, af, ret, buf, buflen, result, h_errnop)  \
  ({ *(result) = gethostbyname2(name, af); 0; })
#endif // __APPLE__

namespace os {

// Checks if the specified key is in the environment variables.
inline bool hasenv(const std::string& key)
{
  char* value = ::getenv(key.c_str());

  return value != NULL;
}


// Looks in the environment variables for the specified key and
// returns a string representation of it's value. If 'expected' is
// true (default) and no environment variable matching key is found,
// this function will exit the process.
inline std::string getenv(const std::string& key, bool expected = true)
{
  char* value = ::getenv(key.c_str());

  if (expected && value == NULL) {
    std::cerr << "Expecting '" << key << "' in environment" << std::endl;
    abort();
  }

  if (value != NULL) {
    return std::string(value);
  }

  return std::string();
}


// Sets the value associated with the specfied key in the set of
// environment variables.
inline void setenv(const std::string& key,
                   const std::string& value,
                   bool overwrite = true)
{
  ::setenv(key.c_str(), value.c_str(), overwrite ? 1 : 0);
}


// Unsets the value associated with the specfied key in the set of
// environment variables.
inline void unsetenv(const std::string& key)
{
  ::unsetenv(key.c_str());
}


inline Try<int> open(const std::string& path, int oflag, mode_t mode = 0)
{
  int fd = ::open(path.c_str(), oflag, mode);

  if (fd < 0) {
    return Try<int>::error(strerror(errno));
  }

  return fd;
}


inline Try<bool> close(int fd)
{
  if (::close(fd) != 0) {
    return Try<bool>::error(strerror(errno));
  }

  return true;
}


inline std::list<std::string> ls(const std::string& directory)
{
  std::list<std::string> entries;

  DIR* dir = opendir(directory.c_str());

  if (dir == NULL) {
    return std::list<std::string>();
  }

  // Calculate the size for a "directory entry".
  long name_max = fpathconf(dirfd(dir), _PC_NAME_MAX);

  // If we don't get a valid size, check NAME_MAX, but fall back on
  // 255 in the worst case ... Danger, Will Robinson!
  if (name_max == -1) {
    name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
  }

  size_t name_end = (size_t) offsetof(dirent, d_name) + name_max + 1;

  size_t size = (name_end > sizeof(dirent) ? name_end : sizeof(dirent));

  dirent* temp = (dirent*) malloc(size);

  if (temp == NULL) {
    free(temp);
    closedir(dir);
    return std::list<std::string>();
  }

  dirent* entry;

  int error;

  while ((error = readdir_r(dir, temp, &entry)) == 0 && entry != NULL) {
    entries.push_back(entry->d_name);
  }

  free(temp);
  closedir(dir);

  if (error != 0) {
    return std::list<std::string>();
  }

  return entries;
}


inline bool exists(const std::string& path, bool directory = false)
{
  struct stat s;

  if (::stat(path.c_str(), &s) < 0) {
    return false;
  }

  // Check if it's a directory if requested.
  return directory ? S_ISDIR(s.st_mode) : true;
}


inline std::string basename(const std::string& path)
{
  return ::basename(const_cast<char*>(path.c_str()));
}


inline Try<std::string> realpath(const std::string& path)
{
  char temp[PATH_MAX];
  if (::realpath(path.c_str(), temp) == NULL) {
    return Try<std::string>::error(
        "Failed to canonicalize %s into an absolute path: %s",
        path.c_str(), strerror(errno));
  }
  return std::string(temp);
}


inline Try<void> mkdir(const std::string& directory)
{
  const std::vector<std::string>& tokens = strings::split(directory, "/");

  std::string path = "";

  // We got an absolute path, so keep the leading slash.
  if (directory.find_first_of("/") == 0) {
    path = "/";
  }

  std::vector<std::string>::const_iterator iterator = tokens.begin();
  while (iterator != tokens.end()) {
    const std::string& token = *iterator;
    path += token;
    if (::mkdir(path.c_str(), 0755) < 0 && errno != EEXIST) {
      return Try<void>::error("mkdir: %s: %s", path.c_str(), strerror(errno));
    }
    path += "/";
    ++iterator;
  }

  return Try<void>::value();
}


inline Try<void> rm(const std::string& path, bool recursive = false)
{
  if (recursive) {
    if (exists(path, true)) {
      const std::list<std::string>& entries = ls(path);
      std::list<std::string>::const_iterator iterator = entries.begin();
      while (iterator != tokens.end()) {
        const std::string& entry = *iterator;
        Try<void> result = rm(entry);
        if (result.isError()) {
          return Try<void>::error(result.error());
        }
        ++iterator;
      }
    } else {
      return Try<void>::error("rm: " + path + ": is a directory");
    }
  }

  if (::unlink(path.c_str()) != 0) {
    return Try<void>::error("rm: %s: %s", path.c_str(), strerror(errno));
  }

  return Try<void>::value();
}


// Changes the specified path's user AND group ownership.
inline Try<void> chown(const std::string& user, const std::string& path)
{
  passwd* passwd;
  if ((passwd = ::getpwnam(user.c_str())) == NULL) {
    return Try<void>::error(strerror(errno));
  }

  if (::chown(path.c_str(), passwd->pw_uid, passwd->pw_gid) < 0) {
    return Try<void>::error(strerror(errno));
  }

  return Try<void>::value();
}


inline Try<void> chdir(const std::string& directory)
{
  if (::chdir(directory.c_str()) < 0) {
    return Try<void>::error("cd: %s: %s", path.c_str(), strerror(errno));
  }

  return Try<void>::value();
}


inline Try<void> su(const std::string& user)
{
  passwd* passwd;
  errno = 0; // Required semantics to distinguish missing from error.
  if ((passwd = ::getpwnam(user.c_str())) == NULL) {
    if (errno != 0) {
      return Try<void>::error("su: %s: %s", user.c_str(), strerror(errno));
    } else {
      return Try<void>::error("su: " + user + " does not exist");
    }
  }

  if (::setgid(passwd->pw_gid) < 0) {
    return Try<void>::error("su: %s: %s", user.c_str(), strerror(errno));
  }

  if (::setuid(passwd->pw_uid) < 0) {
    return Try<void>::error("su: %s: %s", user.c_str(), strerror(errno));
  }

  return Try<void>::value();
}


inline Try<std::string> pwd()
{
  size_t size = 100;

  while (true) {
    char* temp = new char[size];
    if (::getcwd(temp, size) == temp) {
      std::string result(temp);
      delete[] temp;
      return result;
    } else {
      if (errno != ERANGE) {
        delete[] temp;
        return Try<std::string>::error("pwd: %s", strerror(errno));
      }
      size *= 2;
      delete[] temp;
    }
  }

  abort(); // Unreachable code.
}


inline std::string whoami()
{
  passwd* passwd;
  errno = 0; // Required semantics to distinguish missing from error.
  if ((passwd = ::getpwuid(::getuid())) == NULL) {
    if (errno != 0) {
      return Try<void>::error("whoami: %s", strerror(errno));
    } else {
      return Try<void>::error("whoami: current running user does not exist!");
    }
  }

  return passwd->pw_name;
}


inline Try<std::string> hostname()
{
  char host[512];

  if (::gethostname(host, sizeof(host)) < 0) {
    return Try<std::string>::error("hostname: %s", strerror(errno));
  }

  struct hostent he, *hep;
  char* temp;
  size_t length;
  int result;
  int herrno;

  // Allocate temporary buffer for gethostbyname2_r.
  length = 1024;
  temp = new char[length];

  while ((result = ::gethostbyname2_r(
              host, AF_INET, &he, temp, length, &hep, &herrno)) == ERANGE) {
    // Enlarge the buffer.
    delete[] temp;
    length *= 2;
    temp = new char[length];
  }

  if (result != 0 || hep == NULL) {
    delete[] temp;
    return Try<std::string>::error("hostname: %s", hstrerror(herrno));
  }

  std::string hostname = hep->h_name;
  delete[] temp;
  return Try<std::string>::value(hostname);
}


// Returns the current date in the specified format.
inline Try<std::string> date(const std::string& format = "%Y%m%d%H%M")
{
  time_t rawtime;
  tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char date[128]; // Probably big enough for a date string!
  if (strftime(date, sizeof(date), format.c_str(), timeinfo) == 0) {
    return Try<std::string>::error("Failed to format the date string");
  }

  return date;
}


inline int system(const std::string& command)
{
  return ::system(command.c_str());
}


// Returns the total number of cpus (cores).
inline Try<long> cpus()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}


// Returns the total size of main memory in bytes.
inline Try<long> memory()
{
#ifdef __linux__
  struct sysinfo info;
  if (sysinfo(&info) != 0) {
    return Try<long>::error(strerror(errno));
  }
  return info.totalram;
#else
  return Try<long>::error("Cannot determine the size of main memory");
#endif
}

} // namespace os {

#endif // __STOUT_OS_HPP__
