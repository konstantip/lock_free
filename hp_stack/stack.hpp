#pragma once

#include <atomic>
#include <memory>

#include <hp.hpp>

namespace lock_free
{

template <typename T>
class Stack
{
  struct Node final
  {
    std::unique_ptr<T> data;
    Node* next{};

    explicit Node(std::unique_ptr<T> data) : data{std::move(data)} {}
  };

  std::atomic<Node*> head_{};
  std::atomic<Node*> nodes_to_reclame_{};

 public:

  void push(T data)
  {
    pushReadyData(std::make_unique<T>(std::move(data)));
  }

  void pushReadyData(std::unique_ptr<T> data)
  {
    pushNode(new Node{std::move(data)});
  }

  std::unique_ptr<T> pop() noexcept
  {
    std::atomic<void*>& hp = hazard_pointers::getHazardPointerForCurrentThread();

    Node* old_head = head_.load();

    do
    {
      Node* temp;
      do 
      {
        temp = old_head;
        hp.store(old_head);
        old_head = head_.load();
      }
      while (temp != old_head);  //It might be ub, but ok for us
    } 
    while (old_head && !head_.compare_exchange_strong(old_head, old_head->next));
    
    if (!old_head)
    {
      return {};
    }

    std::unique_ptr data = std::move(old_head->data);

    if (!hazard_pointers::otherHazardPoints(old_head))
    {
      delete old_head;
    }
    else
    {
      hazard_pointers::addToReclaimList(old_head);
    }
    
    hazard_pointers::reclaimIfPossible();

    return data;
  }

 private:

  void pushNode(Node* const node) noexcept
  {
    node->next = head_.load();

    while (!head_.compare_exchange_weak(node->next, node));
  }
};

}
