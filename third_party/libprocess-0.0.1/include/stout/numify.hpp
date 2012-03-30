#ifndef __STOUT_NUMIFY_HPP__
#define __STOUT_NUMIFY_HPP__

template <typename T>
Try<T> numify(const std::string& s)
{
  T t;
  std::istringstream in(s);
  in >> t;
  if (in.fail()) {
    return Try::error("Failed to numify '%s'", s.c_str());
  }
  return t;
}

#endif // __STOUT_NUMIFY_HPP__
