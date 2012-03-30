#include <stout/os.hpp>

int main (int argc, char** argv)
{
  std::cerr << "Usage: " << os::basename(argv[0]) << std::endl;
  return 0;
}
