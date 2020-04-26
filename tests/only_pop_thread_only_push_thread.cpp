#include <functional>
#include <iostream>
#include <thread>

#include <stack.hpp>

constexpr int num_of_els{3000000};

void pushMulty(lock_free::Stack<int>& stack)
{
  std::cout << "Start pushing\n";
  for (int i = 0; i < num_of_els; ++i)
  {
    stack.push(i);
  }
  std::cout << "Finish pushing\n";
}

void popMulty(lock_free::Stack<int>& stack)
{
  std::cout << "Start popping\n";

  bool check[num_of_els]{};

  for (int i = 0; i < num_of_els;)
  {
    const auto ptr = stack.pop();
    if (ptr)
    {
      ++i;
      check[*ptr] = true;
    }
    else
    {
      //std::cout << "Sorry " + std::to_string(i) + "\n";
    }
  }

  for (int i = 0; i < num_of_els; ++i)
  {
    if (!check[i])
    {
      std::cout << "Bad check for " + std::to_string(i) + "\n";
      return;
    }
  }

  std::cout << "Finish popping\n";
}

int main()
{
  lock_free::Stack<int> st;

  std::thread pop{&popMulty, std::ref(st)};
  std::this_thread::yield();
  pushMulty(st);

  pop.join();

  return 0;
}
