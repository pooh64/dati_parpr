#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cassert>

#include <iostream>
#include <chrono>

struct Matrix {
	uint32_t const n_row;
	uint32_t const n_col;
	float *buf;

	Matrix(uint32_t n_row_, uint32_t n_col_) :
		n_row(n_row_), n_col(n_col_)
	{
		buf = new float[n_row * n_col];
	}

	float       *operator[](uint32_t row)
		{ return &buf[n_col * row]; }

	float const *operator[](uint32_t row) const
		{ return &buf[n_col * row]; }

	void rand_init()
	{
		srand(time(NULL));
		for (uint32_t i = 0; i < n_row * n_col; ++i)
			buf[i] = rand() * 1.0;
	}
};

void mult_assert(Matrix const &m1, Matrix const &m2, Matrix &res)
{
	assert(m1.n_col == m2.n_row);
	assert(m1.n_row == m2.n_col);
	assert(res.n_row == m1.n_row);
	assert(res.n_col == m2.n_col);
}

void mult_simple(Matrix const &m1, Matrix const &m2, Matrix &res)
{
	mult_assert(m1, m2, res);

	for (uint32_t i = 0; i < res.n_row; ++i) {
		for (uint32_t k = 0; k < res.n_col; ++k) {
			res[i][k] = 0;
			for (uint32_t n = 0; n < m1.n_col; ++n)
				res[i][k] += m1[i][n] * m2[n][k];
		}
	}
}

void mult_byrows(Matrix const &m1, Matrix const &m2, Matrix &res)
{
	mult_assert(m1, m2, res);

	for (uint32_t i = 0; i < res.n_row; ++i) {
		for (uint32_t n = 0; n < res.n_col; ++n)
			res[i][n] = 0;
		for (uint32_t k = 0; k < m2.n_row; ++k) {
			for (uint32_t n = 0; n < res.n_col; ++n)
				res[i][n] += m1[i][n] * m2[k][n];
		}
	}
}

#define MEASURE_TIME(expr, info) do {				\
	auto start  = std::chrono::system_clock::now();		\
	expr;							\
	auto finish = std::chrono::system_clock::now();		\
	auto result = std::chrono::duration_cast		\
		<std::chrono::milliseconds>(finish - start);	\
	std::cout << info << ": " << result.count() << "ms\n";	\
} while (0)


int main(int argc, char **argv)
{
	Matrix  m1(1024, 1024);
	Matrix  m2(1024, 1024);
	Matrix res(1024, 1024);
	m1.rand_init();
	m2.rand_init();
	MEASURE_TIME(mult_simple(m1, m2, res), "simple");
	MEASURE_TIME(mult_byrows(m1, m2, res), "byrows");
	return 0;
}
