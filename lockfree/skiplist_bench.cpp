#include "lf_skiplist.hpp"
#include <vector>
#include <thread>

using test_skiplist = lf_skiplist<int, int, 10>;

void stress_routine(test_skiplist *sl, size_t sz)
{
	for (size_t i = 0; i < sz; ++i) {
		int val = rand();
		if (val % 100 < 5)
			sl->add(rand());
		else if (val % 100 < 20)
			sl->remove(rand());
		else
			sl->contains(rand());
	}
}

void stress_test(int n_thr, size_t sz)
{
	test_skiplist sl;
	std::vector<std::thread> thr;

	for (int i = 0; i < n_thr; ++i)
		thr.push_back(std::thread(stress_routine, &sl, sz));

	for (auto &t: thr)
		t.join();
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		std::cout << "argv\n";
		return 1;
	}

	int n_thr = atoi(argv[1]);
	size_t sz = 1024L * 1024L/ n_thr;
	stress_test(n_thr, sz);

/*	test_skiplist sl;

	std::cout << sl.contains(1) << "\n";
	std::cout << sl.add(1) << "\n";
	std::cout << sl.add(1) << "\n";

	std::cout << sl.contains(1) << "\n";
	std::cout << sl.add(3) << "\n";

	std::cout << sl.contains(3) << "\n";
	std::cout << sl.remove(3) << "\n";
	std::cout << sl.remove(2) << "\n";

	std::cout << sl.add(2) << "\n";
	std::cout << sl.contains(3) << "\n";
*/
	return 0;
}
