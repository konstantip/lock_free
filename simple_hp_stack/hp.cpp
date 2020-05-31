#include <hp.hpp>

constexpr int max_nuf_of_threads{128};

namespace hazard_pointers
{

namespace
{

struct HazardPointer
{
  std::atomic<std::thread::id> thread_id{};
  std::atomic<void*> pointer{};
};

HazardPointer pointers[max_nuf_of_threads];

class HPOwner final
{
  int index{};
 public:
  HPOwner()
  {
    for (int i = 0; i < max_nuf_of_threads; ++i)
    {
      std::thread::id id{};
      if (pointers[i].thread_id.compare_exchange_strong(id, std::this_thread::get_id(), 
                                                        std::memory_order_relaxed))
      {
        index = i;
        return;
      }
    }
    throw NoFreeHazardPointer{};
  }

  ~HPOwner()
  {
    pointers[index].pointer.exchange(nullptr, std::memory_order_relaxed);
    pointers[index].thread_id.exchange(std::thread::id{}, std::memory_order_relaxed);
  }

  std::atomic<void*>& getPointer() const noexcept
  {
    return pointers[index].pointer;
  }
};

}

std::atomic<void*>& getHazardPointerForCurrentThread()
{
  static thread_local const HPOwner this_thread_hp{};

  return this_thread_hp.getPointer();
}

bool otherHazardPoints(const void* const p) noexcept
{
  for (int i = 0; i < max_nuf_of_threads; ++i)
  {
    if (p == pointers[i].pointer.fetch_add(std::memory_order_relaxed))
    {
      return true;
    }
  }

  return false;
}

using ReclaimList = detail::ReclaimList;

void ReclaimList::addNode(Node* const node) noexcept
{
  node->next = head_.load(std::memory_order_relaxed);

  while (!head_.compare_exchange_weak(node->next, node, 
                                      std::memory_order_release, 
                                      std::memory_order_relaxed));
}

void ReclaimList::reclaimIfPossible() noexcept
{
  Node* old_head = head_.exchange(nullptr, std::memory_order_acquire);

  for (; old_head;)
  {
    Node* next = old_head->next;

    if (!otherHazardPoints(old_head))
    {
      delete old_head;
    }
    else
    {
      addNode(old_head);
    }

    old_head = next;
  }
}

ReclaimList::~ReclaimList()
{
  for (Node* old_head = head_.load(); old_head;)
  {
    auto next = old_head->next;
    delete old_head;
    old_head = next;
  }
}

}
