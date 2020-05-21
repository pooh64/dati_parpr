#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include "lf_list.hpp"


void stress_test_producer(lf_list<int> *l, size_t sz)
{
	std::cout << "producer started\n";
	for (size_t i = 0; i < sz; ++i) {
		l->insert(rand());
	}
	std::cout << "producer finished\n";
}

void stress_test_remover(lf_list<int> *l, size_t sz)
{
	std::cout << "remover started\n";
	for (size_t i = 0; i < sz; ++i) {
		l->remove(rand());
	}
	std::cout << "remover finished\n";
}

void stress_test_searcher(lf_list<int> *l, size_t sz)
{
	std::cout << "searcher started\n";
	for (size_t i = 0; i < sz; ++i)
		l->find(rand());
	std::cout << "searcher finished\n";
}

void stress_test(size_t sz)
{
	std::cout << "----stress_test----\n";

	lf_list<int> l;
	std::vector<std::thread> thr;

	for (int i = 0; i < 2; ++i)
		thr.push_back(std::thread(stress_test_producer, &l, sz));

	for (int i = 0; i < 2; ++i)
		thr.push_back(std::thread(stress_test_remover, &l, sz));

	for (int i = 0; i < 1; ++i)
		thr.push_back(std::thread(stress_test_searcher, &l, sz));

	for (auto &v : thr)
		v.join();

	std::cout << "----stress_test passed----\n";
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cout << "WHERE ARE MY ARGUMENTS???\n";
		return 1;
	}

	size_t sz = atol(argv[1]);

	lf_list<int> list;

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

	stress_test(sz);

	return 0;
}
