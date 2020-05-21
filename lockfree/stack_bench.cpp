#include "lf_stack.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <future>

void simple_run()
{
	std::cout << "----simple_run----\n";
	std::vector<int> vec = {1, 2, 3, 4, 5};
	lf_stack<int> stk;
	for (auto const &val : vec)
		stk.push(val);
	stk.dump_unsynchronized(std::cout);

	int val;
	for (long i = vec.size() - 1; i >= 0; --i) {
		if (!stk.pop(&val))
			goto test_failed;
		if (val != vec[i])
			goto test_failed;
	}
	if (stk.pop(&val))
		goto test_failed;

	std::cout << "----simple_run passed----\n";
	return;
test_failed:
	std::cout << "----simple_run failed----\n";
}

#define TRANSFER_TEST_SZ (4 * 1024 * 1024)
#define TRANSFER_TEST_THR (2)

int transfer_test_producer(lf_stack<int> *stk)
{
	std::cout << "producer started\n";
	int sum = 0;
	for (size_t i = 0; i < TRANSFER_TEST_SZ; ++i) {
		int val = rand();
		sum += val;
		stk->push(val);
	}

	std::cout << "producer finished\n";
	return sum;
}

int transfer_test_consumer(lf_stack<int> *stk)
{
	std::cout << "consumer started\n";
	int sum = 0;
	int val;
	for (size_t i = 0; i < TRANSFER_TEST_SZ; ++i) {
		while (!stk->pop(&val))
			__cpu_relax();
		sum += val;
	}

	std::cout << "consumer finished\n";
	return sum;
}

void transfer_test()
{
	std::cout << "----transfer_test----\n";

	std::vector<std::future<int>> produced(TRANSFER_TEST_THR);
	std::vector<std::future<int>> consumed(TRANSFER_TEST_THR);
	int sum_consumed = 0, sum_produced = 0;
	lf_stack<int> stk;

	for (auto &v : consumed)
		v = std::async(transfer_test_consumer, &stk);

	for (auto &v : produced)
		v = std::async(transfer_test_producer, &stk);

	for (auto &v : consumed)
		sum_consumed += v.get();

	for (auto &v : produced)
		sum_produced += v.get();

	std::cout << "sum_consumed = " << sum_consumed
		<< "\nsum_produced = " << sum_produced << "\n";

	if (sum_consumed != sum_produced)
		goto test_failed;
	std::cout << "----transfer_run passed----\n";
        return;

test_failed:
        std::cout << "----transfer_run failed----\n";
}

void stress_routine(lf_stack<int> *stk, size_t sz)
{
	for (size_t i = 0; i < sz; ++i) {
		int val = rand();
		if (val % 10 == 0)
			stk->push(val);
		else
			stk->pop(&val);
	}
}

void stress_test(size_t sz, int n_thr)
{
	std::vector<std::thread> thr;
	lf_stack<int> stk;

	for (int i = 0; i < n_thr; ++i)
		thr.push_back(std::thread(stress_routine, &stk, sz));

	for (auto &t: thr)
		t.join();
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cout << "argv\n";
		return 1;
	}

	size_t sz = 1024L * 1024L;
	int n_thr = atoi(argv[1]);

	//simple_run();
	stress_test(sz, n_thr);
	return 0;
}
