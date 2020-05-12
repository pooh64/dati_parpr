#include "lf_stack.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <future>

bool shared_ptr_test()
{
	std::shared_ptr<int> sptr;
	return std::atomic_is_lock_free(&sptr);
}

void simple_run()
{
	std::cout << "----simple_run----\n";
	std::vector<int> vec = {1, 2, 3, 4, 5};
	lf_stack<int> stk;
	for (auto const &val : vec)
		stk.push(val);
	stk.dump_unsynchronized(std::cout);

	for (long i = vec.size() - 1; i >= 0; --i) {
		auto ptr = stk.pop();
		if (ptr == nullptr)
			goto test_failed;
		if (*ptr != vec[i])
			goto test_failed;
	}
	if (stk.pop() != nullptr)
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
	int sum = 0;
	for (size_t i = 0; i < TRANSFER_TEST_SZ; ++i) {
		int val = rand();
		sum += val;
		stk->push(val);
	}
	return sum;
}

int transfer_test_consumer(lf_stack<int> *stk)
{
	int sum = 0;
	std::shared_ptr<int> ptr;
	for (size_t i = 0; i < TRANSFER_TEST_SZ; ++i) {
		while ((ptr = stk->pop()) == nullptr)
			__cpu_relax();
		sum += *ptr;
	}
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

int main()
{
	std::cout << "shared_ptr lock-free = " << shared_ptr_test() << "\n";
	simple_run();
	transfer_test();
	return 0;
}
