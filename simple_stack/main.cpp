#include "stack.hpp"

#include <iostream>

int main()
{
  LockFreeStack<int> st;

  st.push(50);

  st.push(89);

  auto ptr = st.pop();

  if (ptr)
    std::cout << *ptr << std::endl;

  return 0;
}
