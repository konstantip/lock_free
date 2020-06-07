#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <type_traits>

namespace hazard_pointers
{

class NoFreeHazardPointer : public std::exception
{
  using exception::exception;
};

namespace detail
{

class ReclaimList final
{
  class Node final
  {
   private:
    template <typename T>
    static void deleteHelper(const void* const data)
    {
      delete static_cast<const T*>(data);
    }

    std::function<void(const void*)> deleter;
    const void* const data;

   public:
    Node* next{};

    template <typename T>
    Node(const T* const data) noexcept : deleter{&deleteHelper<T>}, data{data} {}

    ~Node()
    {
      deleter(data);
    }
  };

  std::atomic<Node*> head_{};

  void addNode(Node* const node) noexcept;
 public:
  
  template <typename T>
  void add(const T* const data)
  {
    Node* new_node = new Node{data};
    addNode(new_node);
  }

  void reclaimIfPossible() noexcept;

  ~ReclaimList();
};

inline ReclaimList reclaim_list{};

}


bool otherHazardPoints(const void* const p) noexcept;
std::atomic<void*>& getHazardPointerForCurrentThread();

template <typename T>
void addToReclaimList(const T* const data) noexcept
{
  static_assert(std::is_nothrow_destructible_v<T>, "destructor of T must not throw");
  for (;;)
  {
    try
    {
      detail::reclaim_list.add(data);
      break;
    }
    catch (const std::bad_alloc&)
    {
      if (!otherHazardPoints(data))
      {
        delete data;
        break;
      }
    }
  }
}

inline void reclaimIfPossible() noexcept
{
  getHazardPointerForCurrentThread().exchange(nullptr, std::memory_order_relaxed);
  detail::reclaim_list.reclaimIfPossible();
}

}
