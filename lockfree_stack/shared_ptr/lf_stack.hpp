#pragma once
#include <atomic>
#include <memory>
#include <iostream>

#define __cpu_relax() do { asm volatile("pause"); } while (0)

template <typename T>
class lf_stack {
private:
	struct node {
		std::shared_ptr<T> data;
		std::shared_ptr<node> next;
		node(T const &data_) : data(std::make_shared<T>(data_)) {}
	};

	std::shared_ptr<node> head;

public:
	void push(T const &data)
	{
		std::shared_ptr<node> const new_node =
			std::make_shared<node>(data);
		new_node->next = std::atomic_load(&head);
		while (!std::atomic_compare_exchange_weak_explicit(
				&head, &new_node->next, new_node,
				std::memory_order_release, std::memory_order_relaxed))
			__cpu_relax();
	}

	std::shared_ptr<T> pop()
	{
		std::shared_ptr<node> old_head = std::atomic_load(&head);
		while (old_head && !std::atomic_compare_exchange_weak_explicit(
				&head, &old_head, old_head->next,
				std::memory_order_acquire, std::memory_order_relaxed))
			__cpu_relax();
		return old_head ? old_head->data : std::shared_ptr<T>();
	}

	std::ostream& dump_unsynchronized(std::ostream& os)
	{
		os << "----lf_stack.dump_unsynchronized----\n";
		std::shared_ptr<node> ptr = head;
		size_t counter = 0;
		while (ptr) {
			os << "[" << counter << "] = " << *(ptr->data) << "\n";
			ptr = ptr->next; counter++;
		}
		os << "--------\n";
		return os;
	}

	template<class UnaryPredicate>
	std::shared_ptr<T> search(UnaryPredicate p)
	{
		std::shared_ptr<node> ptr = head;
		while (ptr) {
			if (p(ptr->data))
				return ptr;
			ptr = ptr->next;
		}
		return std::shared_ptr<T>();
	}
};
