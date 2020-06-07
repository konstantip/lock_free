#include <iostream>
#include <cstring>

#include <stack.hpp>

int main(int argc, char* argv[])
{
  const bool verbose = [argc, argv]{
    for (int i = 1; i < argc; ++i)
    {
      if (!strcmp(argv[i], "--verbose"))
      {
        return true;
      }
    }
    return false;
  }();

  lock_free::Stack<int> stack;

  const bool is_lock_free = stack.is_lock_free();

  if (verbose)
  {
    if (is_lock_free)
    {
      std::cout << "Container is lock-free on current platform! :)" << std::endl;
    }
    else
    {
      std::cout << "Container is not lock-free on current platform :(" <<std::endl;
    }
  }

  return !is_lock_free;
}
