#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

namespace lock_free
{

template <typename T>
class Stack
{
  struct Node;

  struct NodePtr final
  {
    Node* node{};
    unsigned external_counter{};

    static NodePtr CreateNodePtr(Node* const node) noexcept
    {
      return NodePtr{node, 1};
    }
    static NodePtr CreateNodePtr(T&& data)
    {
      return CreateNodePtr(new Node{std::make_unique<T>(std::move(data))});
    }
    static NodePtr CreateNodePtr(const T& data) 
    {
      return CreateNodePtr(new Node{std::make_unique<T>(data)});
    }
    static NodePtr CreateNodePtr(std::unique_ptr<T> data)
    {
      return CreateNodePtr(new Node{std::move(data)});
    }
  };

  struct Node final
  {
    std::unique_ptr<T> data;
    std::atomic<int> internal_counter;
    NodePtr next;

    Node(std::unique_ptr<T> ptr) noexcept : data{std::move(ptr)}, internal_counter{}, next{} {}
  };

  std::atomic<NodePtr> head_{};

 public:
  void push(T&& data)
  {
    pushNode(NodePtr::CreateNodePtr(std::move(data)));
  }

  void push(const T& data)
  {
    pushNode(NodePtr::CreateNodePtr(data));
  }

  void pushReadyData(std::unique_ptr<T> data)
  {
    pushNode(NodePtr::CreateNodePtr(std::move(data)));
  }

  std::unique_ptr<T> pop() noexcept
  {
    NodePtr old_head = head_.load(std::memory_order_relaxed);
    for (;;)
    {
      incrementHeadCounter(old_head);

      if (!old_head.node)
      {
        return {};
      }

      Node* const node = old_head.node;

      if (head_.compare_exchange_strong(old_head, node->next, std::memory_order_relaxed))
      {
        std::unique_ptr data = std::move(node->data);

	const int external_count = old_head.external_counter - 2;
	if (node->internal_counter.fetch_add(external_count, 
                                             std::memory_order_release) == 
            -external_count)
	{
          delete node;
        }
        
        return data;
      }

      if (node->internal_counter.fetch_sub(1, std::memory_order_relaxed) == 1)
      {
        node->internal_counter.load(std::memory_order_acquire);
        delete node;
      }
    }
  }

 private:
  void pushNode(NodePtr node_ptr) noexcept
  {
    node_ptr.node->next = head_.load(std::memory_order_relaxed);

    while (!head_.compare_exchange_weak(node_ptr.node->next, node_ptr, 
                                        std::memory_order_release, 
                                        std::memory_order_relaxed));
  }

  void incrementHeadCounter(NodePtr& old_head) noexcept
  {
    NodePtr tmp;
    do
    {
      tmp = old_head;
      ++tmp.external_counter;
    }
    while (!head_.compare_exchange_weak(old_head, tmp, 
                                        std::memory_order_acquire, 
                                        std::memory_order_relaxed));

    ++old_head.external_counter;
  }
};

}
