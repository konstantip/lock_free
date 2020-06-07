#pragma once

#include <atomic>
#include <memory>

namespace lock_free
{

template <typename T>
class Stack
{
  struct Node final
  {
    std::shared_ptr<Node> next;
    std::unique_ptr<T> data;

    Node(std::unique_ptr<T> ptr) : data{std::move(ptr)} {}
  };

  std::shared_ptr<Node> head_;

  void pushNode(std::shared_ptr<Node> node) noexcept
  {
    node->next = std::atomic_load_explicit(&head_, std::memory_order_relaxed);

    while (!std::atomic_compare_exchange_weak_explicit(&head_, &node->next, node, 
                                                       std::memory_order_release, 
                                                       std::memory_order_relaxed));
  }

 public:
  void push(T&& data)
  {
    pushReadyData(std::make_unique<T>(std::move(data)));
  }

  void push(const T& data)
  {
    pushReadyData(std::make_unique<T>(std::move(data)));
  }

  void pushReadyData(std::unique_ptr<T> ptr) noexcept
  {
    pushNode(std::make_shared<Node>(std::move(ptr)));
  }

  std::unique_ptr<T> pop() noexcept
  {
    auto old_head = std::atomic_load_explicit(&head_, std::memory_order_relaxed);

    while (old_head && !std::atomic_compare_exchange_weak_explicit(&head_, &old_head, old_head->next, 
                                                                  std::memory_order_acquire, 
                                                                  std::memory_order_relaxed));

    if (!old_head)
    {
      return {};
    }

    return std::move(old_head->data);
  }

  bool isLockFree() const noexcept
  {
    return std::atomic_is_lock_free(&head_);
  }
};

}
