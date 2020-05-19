#include <iostream>
#include "lf_ordered_list.hpp"

int main()
{
	lf_ordered_list<int> list;

	list.insert(1);
	list.insert(2);
	list.insert(3);

	list.insert(2);
	list.insert(5);
	list.insert(4);

	list.remove(2);
	list.remove(3);
	list.remove(1);

	list.remove(4);
	list.remove(5);

	list.dummy_dump();

	list.free_list.dump_unsynchronized(std::cout);

	return 0;
}
