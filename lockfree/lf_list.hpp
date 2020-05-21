#include <atomic>
#include <iostream>
#include "tag_ptr.h"
#include "lf_stack.hpp"

template <typename T>
struct lf_list {
	struct node {
		T key;
		std::atomic<tag_ptr<node>> next;

		node(T const &key_) : key(key_) {
			tag_ptr<node> dummy; dummy.set(nullptr, 0);
			next = dummy;
		}
		node(const node &v) : key(v.key), next(v.next.load()) { }
		node() { }
		node& operator=(const node& v) {
			key = v.key; next = v.next.load(); }
	};

	tag_ptr<node> head;
	tag_ptr<node> tail;
	lf_stack<node*> free_list;

	lf_list()
	{
		head.set(new node(0), 0);
		tail.set(new node(0), 0);
		head->next = tail;
	}

	void clear_free_list()
	{
		node *ptr;
		while (free_list.pop(&ptr))
			delete ptr;
	}

	~lf_list()
	{
		clear_free_list();

		for (tag_ptr<node> ptr = head; ptr.get_ptr() != nullptr;) {
			tag_ptr<node> next = ptr->next;
			delete ptr.get_ptr();
			ptr = next;
		}
	}

	tag_ptr<node> search(T const &search_key, tag_ptr<node> *left_node)
	{
		tag_ptr<node> left_node_next, right_node;

	search_again:
		do {
			tag_ptr<node> t = head;
			tag_ptr<node> t_next = head->next;

			do {
				if (!t_next.is_marked()) {
					(*left_node) = t;
					left_node_next = t_next;
				}
				t = t_next.get_unmarked();
				if (t == tail) break;
				t_next = t->next;
			} while (t_next.is_marked() || (t->key < search_key));

			right_node = t;

			if (left_node_next == right_node) {
				if ((right_node != tail) &&
						(right_node->next).load().is_marked())
					goto search_again;
				else
					return right_node;
			}

			if (((*left_node)->next).compare_exchange_strong(
					left_node_next, right_node.get_inc())) {
				if ((right_node != tail) &&
						(right_node->next).load().is_marked())
					goto search_again;
				else
					return right_node;
			}
		} while (true);
	}

	bool insert(T const &key)
	{
		tag_ptr<node> new_node, right_node, left_node;
		new_node.set(new node(key), 0);

		do {
			right_node = search(key, &left_node);
			if ((right_node != tail) && (right_node->key == key)) {
				delete new_node.get_ptr();
				return false;
			}
			new_node->next = right_node;
			if ((left_node->next).compare_exchange_strong(right_node,
						new_node.get_inc()))
				return true;
		} while (1);
	}

	bool find(T const &search_key)
	{
		tag_ptr<node> right_node, left_node;

		right_node = search(search_key, &left_node);
		if ((right_node == tail) || (right_node->key != search_key))
			return false;
		else
			return true;
	}

	bool remove(T const &search_key)
	{
		tag_ptr<node> right_node, right_node_next, left_node;
		do {
			right_node = search(search_key, &left_node);
			if ((right_node == tail) ||
					(right_node->key != search_key))
				return false;
			right_node_next = right_node->next;
			if (!right_node_next.is_marked()) {
				if ((right_node->next).compare_exchange_strong(
						right_node_next,
						right_node_next.get_marked().get_inc()))
					break;
			}
		} while (true);
		if (!(left_node->next).compare_exchange_strong(right_node, right_node_next.get_inc()))
			right_node = search(right_node->key, &left_node);
		else
			free_list.push(right_node.get_ptr());
		return true;
	}

	void dummy_dump()
	{
		tag_ptr<node> ptr = head->next;
		std::cout << "[head]\n";
		while (ptr.get_ptr() != tail.get_ptr()) {
			std::cout << "[" << ptr->key << "]\n";
			ptr = ptr->next;
		}
		std::cout << "[tail]\n";
	}
};
