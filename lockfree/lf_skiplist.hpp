#pragma once
#include "lf_list.hpp"
#include <cstdint>
#include <limits>

template <typename K, typename V, uint8_t L>
struct lf_skiplist {
	struct node {
		K key;
		V val;
		int top_level;
		std::atomic<tag_ptr<node>> next[L];

		node(K const &key_)
			: key(key_), top_level(L-1)
		{
			tag_ptr<node> dummy; dummy.set(nullptr, 0);
			for (int i = 0; i < L; ++i)
				next[i] = dummy;
		}

		node(V const &val_, int height_)
			: val(val_), top_level(height_)
		{
			key = val; // dummy hash
			tag_ptr<node> dummy; dummy.set(nullptr, 0);
			for (int i = 0; i < L; ++i)
				next[i] = dummy;
		}
	};

	tag_ptr<node> head, tail;

	lf_stack<tag_ptr<node>> freelist;

	lf_skiplist()
	{
		head.set(new node(std::numeric_limits<K>::min()), 0);
		tail.set(new node(std::numeric_limits<K>::max()), 0);
		for (int i = 0; i < L; ++i)
			head->next[i] = tail;
	}

	~lf_skiplist()
	{
		//head = head.get_unmarked();
		//tail = tail.get_unmarked();
		//delete head.get_ptr();
		//delete tail.get_ptr();

		tag_ptr<node> ptr = head;
		while (ptr) {
			node *tmp = ptr.get_unmarked().get_ptr();
			ptr = ptr.get_unmarked()->next[0];
			delete tmp;
		}

		while (freelist.pop(&ptr)) {
			ptr = ptr.get_unmarked();
			delete ptr.get_ptr();
		}
	}

	uint8_t uint32_lsb(uint32_t val)
	{
		for (uint8_t i = 0; i < 32; ++i) {
			if ((val >> i) & (uint32_t) 1)
				return i;
		}
		return 32;
	}

	uint8_t random_level()
	{
		uint8_t val = uint32_lsb(rand() % ((1 << L) - 1) + 1);
		return val;
	}

	bool find(V const &val, tag_ptr<node> *preds, tag_ptr<node> *succs)
	{
		int bot_level = 0;
		int key = val; // dummy hash
		bool marked = false;
		tag_ptr<node> pred, curr, succ;
		pred.set(nullptr, 0);
		curr.set(nullptr, 0);
		succ.set(nullptr, 0);
	find_again:
		while (1) {
			pred = head;
			for (int level = L-1; level >= bot_level; --level) {
				curr = pred->next[level].load().get_unmarked();
				while (1) {
					succ = curr->next[level];
					marked = succ.is_marked();
					succ = succ.get_unmarked();
					while (marked) {
						curr = curr.get_unmarked();
						if (!(pred->next[level]).compare_exchange_strong(curr,
								succ.get_unmarked()))
							goto find_again;
						curr = pred->next[level].load().get_unmarked();
						succ = curr->next[level];
						marked = succ.is_marked();
						succ = succ.get_unmarked();
					}
					if (curr->key < key) {
						pred = curr; curr = succ;
					} else {
						break;
					}
				}
				preds[level] = pred;
				succs[level] = curr;
			}
			return (curr->key == key);
		}
	}


	bool add(V const &val)
	{
		int top_level = random_level();
		int bot_level = 0;
		tag_ptr<node> preds[L], succs[L];
		tag_ptr<node> new_node;
		new_node.set(new node(val, top_level), 0);

		while (1) {
			if(find(val, preds, succs)) {
				delete new_node.get_unmarked().get_ptr();
				return false;
			}
			new_node.set(new (new_node.get_unmarked().get_ptr()) node(val, top_level), 0);
			for (int level = bot_level; level <= top_level; ++level) {
				tag_ptr<node> succ = succs[level];
				new_node->next[level] = succ.get_unmarked();
			}
			tag_ptr<node> pred = preds[bot_level];
			tag_ptr<node> succ = succs[bot_level];
			succ = succ.get_unmarked();
			new_node->next[bot_level] = succ;
			if (!pred->next[bot_level].compare_exchange_strong(succ, new_node))
				continue;
			for (int level = bot_level+1; level < top_level; ++level) {
				while (true) {
					pred = preds[level].get_unmarked();
					succ = succs[level].get_unmarked();
					if (pred->next[level].compare_exchange_strong(
								succ, new_node))
						break;
					find(val, preds, succs);
				}
			}
			return true;
		}
	}

	/* Warning: expect threated as reference only */
	void try_mark(std::atomic<tag_ptr<node>> &target, tag_ptr<node> expect,
			bool new_mark)
	{
		tag_ptr<node> new_val;
		if (new_mark) {
			expect = expect.get_unmarked();
			new_val = expect.get_marked();
		} else {
			expect = expect.get_marked();
			new_val = expect.get_unmarked();
		}
		target.compare_exchange_strong(expect, new_val);
	}

	bool remove(V const &val)
	{
		int bot_level = 0;
		tag_ptr<node> preds[L], succs[L];
		tag_ptr<node> succ;
		while (true) {
			if (!find(val, preds, succs))
				return false;
			tag_ptr<node> node_to_remove = succs[bot_level];
			for (int level = node_to_remove->top_level;
					level >= bot_level+1; --level) {
				succ = node_to_remove->next[level];
				while (!succ.is_marked()) {
					succ = succ.get_unmarked();
					try_mark(node_to_remove->next[level], succ, true);
					succ = node_to_remove->next[level];
				}
			}
			succ = node_to_remove->next[bot_level];
			while (1) {
				succ = succ.get_unmarked();
				bool marked_set = node_to_remove->next[bot_level].
					compare_exchange_strong(succ, succ.get_marked());
				succ = succs[bot_level]->next[bot_level];
				if (marked_set) {
					find(val, preds, succs);
					freelist.push(node_to_remove);
					return true;
				} else if (succ.is_marked()) {
					return false;
				}
			}
		}
	}

	bool contains(V const &val)
	{
		int bot_level = 0;
		K key = val; // dummy hash
		bool marked = false;
		tag_ptr<node> pred, curr, succ;
		pred = head;
		curr.set(nullptr, 0);
		succ.set(nullptr, 0);
		for (int level = L - 1; level >= bot_level; --level) {
			curr = pred->next[level].load();
			curr = curr.get_unmarked();
			while (1) {
				succ = curr->next[level];
				while (succ.is_marked()) {
					succ = curr;
					curr = pred->next[level];
					curr = curr.get_unmarked();
					pred = succ;
					succ = curr->next[level];
				}
				succ = succ.get_unmarked();
				curr = curr.get_unmarked();
				if (curr->key < key) {
					pred = curr;
					curr = succ;
				} else {
					break;
				}
			}
		}
		return (curr->key == key);
	}
};
