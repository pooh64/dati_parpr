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
struct lf_stack {
	struct node {
		T data;
		std::atomic<uint16_t> ref_count;
		tag_ptr<node> next;
		node(T const &data_)
			: data(data_), ref_count(0) {}
	};
	std::atomic<tag_ptr<node>> head;

	void increase_head_tag(tag_ptr<node> &old_head);
	void push(T const &val);
	bool pop(T *res);
	std::ostream &dump_unsynchronized(std::ostream& os);
	template <class UnaryPredicate>
	bool search(UnaryPredicate p, T *res);

	lf_stack()
	{
		tag_ptr<node> dummy; dummy.set_ptr(nullptr); dummy.set_tag(0);
		head.store(dummy);
	}

	~lf_stack()
	{
		int val;
		while (pop(&val));
	}
};

template <typename T>
void lf_stack<T>::increase_head_tag(tag_ptr<node> &old_head)
{
	tag_ptr<node> new_head;
	while (1) {
		new_head = old_head;
		new_head.set_tag(new_head.get_tag() + 1);
		if (head.compare_exchange_strong(old_head, new_head))
			break;
		__cpu_relax();
	}
	old_head.set_tag(new_head.get_tag());
}

template <typename T>
void lf_stack<T>::push(T const &data)
{
	tag_ptr<node> new_node;
	new_node.set_ptr(new node(data));
	new_node.set_tag(1);
	new_node.get_ptr()->next = head.load();
	while (!head.compare_exchange_weak(new_node.get_ptr()->next,
					   new_node))
		__cpu_relax();
}

template <typename T>
bool lf_stack<T>::pop(T *res)
{
	tag_ptr<node> old_head = head.load();
	while (1) {
		increase_head_tag(old_head);
		node *const ptr = old_head.get_ptr();
		if (!ptr) {
			return false;
		}
		if (head.compare_exchange_strong(old_head, ptr->next)) {
			*res = ptr->data;
			int const tag_increase = old_head.get_tag() - 2;
			if (ptr->ref_count.fetch_add(tag_increase)
					== -tag_increase) {
				delete ptr;
			}
			return true;
		} else if (ptr->ref_count.fetch_sub(1) == 1) {
			delete ptr;
		}
		__cpu_relax();
	}
}

template <typename T>
std::ostream& lf_stack<T>::dump_unsynchronized(std::ostream& os)
{
	os << "----lf_stack.dump_unsynchronized----\n";
	node *ptr = head.load().get_ptr();
	size_t counter = 0;
	while (ptr) {
		os << "[" << counter << "] = " << ptr->data << "\n";
		ptr = ptr->next.get_ptr(); counter++;
	}
	os << "--------\n";
	return os;
}

template <typename T>
template <class UnaryPredicate>
bool lf_stack<T>::search(UnaryPredicate p, T *res)
{
	node *ptr = head.get_ptr();
	while (ptr) {
		if (p(ptr->data)) {
			*res = ptr->data;
			return true;
		}
		ptr = ptr->next.get_ptr();
	}
	return false;
}
