#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

namespace lock_free
{

template <typename T>
class Stack
{
  struct Node final
  {
    Node* next{};
    std::unique_ptr<T> data;

    Node(std::unique_ptr<T> data) : data{std::move(data)} {}
  };

 public:
  Stack() = default;
  Stack(const Stack&) = delete;
  Stack& operator=(const Stack&) = delete;

  void push(T data)
  {
    pushReadyData(std::make_unique<T>(std::move(data)));
  }

  void pushReadyData(std::unique_ptr<T> ready_ptr)
  {
    pushNode(new Node{std::move(ready_ptr)});
  }

  std::unique_ptr<T> pop() noexcept
  {
    threads_in_pop_.fetch_add(1, std::memory_order_relaxed);

    auto old_head = head_.load(std::memory_order_relaxed);

    for (; old_head && !head_.compare_exchange_weak(old_head, old_head->next, 
			                            std::memory_order_acquire, 
						    std::memory_order_relaxed); );

    std::unique_ptr data = old_head ? std::move(old_head->data) : nullptr;

    tryClearPossible(old_head);

    return data;
  }

  ~Stack()
  {
    deleteNodes(head_);
    deleteNodes(nodes_to_delete_);
  }

 private:
  void pushNode(Node* const node) noexcept
  {
    node->next = head_.load(std::memory_order_relaxed);

    for (; !head_.compare_exchange_weak(node->next, node, 
			                std::memory_order_release, 
					std::memory_order_relaxed); );
  }

  void tryClearPossible(Node* old_head) noexcept
  {
    if (threads_in_pop_.fetch_add(0, std::memory_order_relaxed) == 1)
    {
      auto nodes_to_delete = head_.exchange(nullptr, std::memory_order_acquire);
      if (nodes_to_delete)
      {
        if (!threads_in_pop_.fetch_add(0, std::memory_order_relaxed))
        {
          deleteNodes(nodes_to_delete);
        }
        else
        {
          addPoppedNodes(nodes_to_delete);
        }
      }

      delete old_head;
    }
    else
    {
      if (old_head)
      {
        addPoppedNode(old_head);
      }

      threads_in_pop_.fetch_sub(1, std::memory_order_relaxed);
    }
  }

  void addPoppedRange(Node* const first, Node* const last) noexcept
  {
    last->next = nodes_to_delete_.load(std::memory_order_seq_cst);

    for (; !nodes_to_delete_.compare_exchange_weak(last->next, first, 
			                           std::memory_order_release, 
						   std::memory_order_relaxed););
  }

  void addPoppedNode(Node* const node) noexcept
  {
    addPoppedRange(node, node);
  }

  void addPoppedNodes(Node* const first) noexcept
  {
    Node* last = first;
    for (; last->next; last = last->next);

    addPoppedRange(first, last);
  }

  void deleteNodes(Node* current) noexcept
  {
    static_assert(std::is_nothrow_destructible_v<T>, "destructor of T must not throw");

    for (Node* next; current; next = current->next, delete current, current = next);
  }

  std::atomic<Node*> head_{};
  std::atomic<Node*> nodes_to_delete_{};

  std::atomic_size_t threads_in_pop_{};
};

}
