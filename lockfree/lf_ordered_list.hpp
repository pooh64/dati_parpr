#include <atomic>
#include "../tag_ptr.h"

template <typename T>
struct lf_ordered_list {
	struct node {
		T data;
		tag_ptr<node> next;
		node(T const &data_)
			: data(data_)
	};
	std::atomic<tag_ptr<node>> head;

	lf_ordered_list()
	{
		head.store(tag_ptr<node>(nullptr, 0));
	}

	void inse
};
