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
    if (p == pointers[i].pointer.load())
    {
      return true;
    }
  }

  return false;
}

using ThreadSafeReclaimList = detail::ThreadSafeReclaimList;

void ThreadSafeReclaimList::addNode(Node* const node) noexcept
{
  node->next = head_.load(std::memory_order_relaxed);

  while (!head_.compare_exchange_weak(node->next, node, 
                                      std::memory_order_release, 
                                      std::memory_order_relaxed));
}

void ThreadSafeReclaimList::addNodes(Node* const head) noexcept
{
  if (!head)
  {
    return;
  }

  Node* tail{head};
  for (Node* next = head->next; next; tail = next, next = next->next);

  tail->next = head_.load(std::memory_order_relaxed);
  while (!head_.compare_exchange_weak(tail->next, head,
                                      std::memory_order_release,
                                      std::memory_order_relaxed));
}

void ThreadSafeReclaimList::reclaimIfPossible() noexcept
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
      add(old_head);
    }

    old_head = next;
  }
}

detail::Node* ThreadSafeReclaimList::exchange() noexcept
{
  return head_.exchange(nullptr, std::memory_order_acquire);
}

ThreadSafeReclaimList::~ThreadSafeReclaimList()
{
  for (Node* old_head = head_.load(); old_head;)
  {
    auto next = old_head->next;
    delete old_head;
    old_head = next;
  }
}

using ReclaimList = detail::ReclaimList;

void ReclaimList::addNode(Node* const node) noexcept
{
  ++size_;
  node->next = head_;
  head_ = node;
}

std::size_t ReclaimList::size() const noexcept
{
  return size_;
}

void ReclaimList::acceptFromGlobal() noexcept
{
  auto global_list = global_reclaim_list.exchange();

  if (!global_list)
  {
    return;
  }

  std::size_t delta_len{};
  const auto global_head = global_list;
  for (auto next = global_list->next; next; global_list = next, next = next->next, ++delta_len);

  global_list->next = head_;
  head_ = global_head;
  size_ += delta_len;
}

void ReclaimList::reclaimIfPossible() noexcept
{
  Node* old_head = head_;
  head_ = nullptr;

  acceptFromGlobal();

  if (size() < max_nuf_of_threads)
  {
    return;
  }

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
  global_reclaim_list.addNodes(head_);
}

}
