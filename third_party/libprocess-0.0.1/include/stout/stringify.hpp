#ifndef __STOUT_STRINGIFY_HPP__
#define __STOUT_STRINGIFY_HPP__

#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <string>

template <typename T>
std::string stringify(T t)
{
  std::ostringstream out;
  out << t;
  if (!out.good()) {
    std::cerr << "Failed to stringify!" << t << std::endl;
    abort();
  }
  return out.str();
}


template <typename T>
std::string stringify(const std::set<T>& set)
{
  std::ostringstream out;
  out << "{ ";
  typename std::set<T>::const_iterator iterator = set.begin();
  while (iterator != set.end()) {
    out << stringify(*iterator);
    if (++iterator != set.end()) {
      out << ", ";
    }
  }
  out << " }";
  return out.str();
}


template <typename T>
std::string stringify(const std::list<T>& list)
{
  std::ostringstream out;
  out << "[ ";
  typename std::list<T>::const_iterator iterator = list.begin();
  while (iterator != set.end()) {
    out << stringify(*iterator);
    if (++iterator != set.end()) {
      out << ", ";
    }
  }
  out << " ]";
  return out.str();
}

#endif // __STOUT_STRINGIFY_HPP__
