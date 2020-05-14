#pragma once

#include "tag_ptr.h"

#include <memory>
#include <atomic>
#include <iostream>

//#define CHILL_RELAX
#ifdef CHILL_RELAX
#define __cpu_relax() do { asm volatile("pause"); } while (0)
#else
#define __cpu_relax() /* DON'T RELAX */
#endif

template <typename T>
class lf_stack {
private:
	struct node {
		std::shared_ptr<T> data;
		std::atomic<uint16_t> ref_count;
		tag_ptr<node> next;
		node(T const &data_)
			: data(std::make_shared<T>(data_)), ref_count(0) {}
	};
	std::atomic<tag_ptr<node>> head;

	void increase_head_ref(tag_ptr<node> &old_ref)
	{
		tag_ptr<node> new_ref;
		while (1) {
			new_ref = old_ref;
			new_ref.set_tag(new_ref.get_tag() + 1);
			if (head.compare_exchange_strong(old_ref, new_ref))
				break;
			__cpu_relax();
		}
		old_ref.set_tag(new_ref.get_tag());
	}

public:
	lf_stack()
	{
		tag_ptr<node> dummy; dummy.set_ptr(nullptr); dummy.set_tag(0);
		head.store(dummy);
	}

	~lf_stack()
	{
		while (pop());
	}

	void push(T const &data)
	{
		tag_ptr<node> new_node;
		new_node.set_ptr(new node(data));
		new_node.set_tag(1);
		new_node.get_ptr()->next = head.load();
		while (!head.compare_exchange_weak(new_node.get_ptr()->next,
						   new_node))
			__cpu_relax();
	}

	std::shared_ptr<T> pop()
	{
		tag_ptr<node> old_head = head.load();
		while (1) {
			increase_head_ref(old_head);
			node *const ptr = old_head.get_ptr();
			if (!ptr) {
				return std::shared_ptr<T>();
			}
			if (head.compare_exchange_strong(old_head, ptr->next)) {
				std::shared_ptr<T> res;
				res.swap(ptr->data);
				int const ref_increase =
					old_head.get_tag() - 2;
				if (ptr->ref_count.fetch_add(ref_increase)
						== -ref_increase) {
					delete ptr;
				}
				return res;
			} else if (ptr->ref_count.fetch_sub(1) == 1) {
				delete ptr;
			}
			__cpu_relax();
		}
	}

	std::ostream& dump_unsynchronized(std::ostream& os)
	{
		os << "----lf_stack.dump_unsynchronized----\n";
		node *ptr = head.load().get_ptr();
		size_t counter = 0;
		while (ptr) {
			os << "[" << counter << "] = " << *(ptr->data) << "\n";
			ptr = ptr->next.get_ptr(); counter++;
		}
		os << "--------\n";
		return os;
	}

	template<class UnaryPredicate>
	std::shared_ptr<T> search(UnaryPredicate p)
	{
		std::shared_ptr<node> ptr = head.get_ptr();
		while (ptr) {
			if (p(ptr->data))
				return ptr;
			ptr = ptr->next.get_ptr();
		}
		return std::shared_ptr<T>();
	}
};
