#include <memory>
#include <mutex>

namespace lock_free
{
template <typename T>
class Stack
{
  struct Node final
  {
    Node* next;
    std::unique_ptr<T> data;

    Node(std::unique_ptr<T> data) : data{std::move(data)} {}
  };

  Node* head_{};
  std::mutex m_;

  //Let's think locking does not throw
  void pushNode(Node* const node) noexcept
  {
    std::lock_guard lk{m_};

    node->next = head_;
    head_ = node;
  }

 public:
  void push(const T& data)
  {
    pushReadyData(std::make_unique<T>(data));
  }

  void push(T&& data)
  {
    pushReadyData(std::make_unique<T>(std::move(data)));
  }

  void pushReadyData(std::unique_ptr<T> data)
  {
    const auto node = new Node{std::move(data)};

    pushNode(node);
  }

  std::unique_ptr<T> pop() noexcept
  {
    const auto old_head = [&]() noexcept -> Node* {
      std::lock_guard lk{m_};

      if (!head_)
      {
        return nullptr;
      }

      const auto old_head = head_;
      head_ = head_->next;
      
      return old_head;
    }();

    if (!old_head)
    {
      return {};
    }

    auto data = std::move(old_head->data);

    delete old_head;

    return data;
  }

  ~Stack()
  {
    while (head_)
    {
      const auto next = head_->next;
      delete head_;
      head_ = next;
    }
  }
};
}
