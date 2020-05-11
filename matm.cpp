#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cassert>
#include <cmath>

#include <iostream>
#include <chrono>

float *alloc_mat(uint32_t n_rows, uint32_t n_cols)
{
	return (float*) malloc(sizeof(float) * n_rows * n_cols);
}

float *gen_random_mat(uint32_t n_rows, uint32_t n_cols)
{
	float *buf = alloc_mat(n_rows, n_cols);
	for (uint32_t i = 0; i < n_rows * n_cols; ++i)
		buf[i] = (float) rand() / RAND_MAX;
	return buf;
}

void mat_print(float *m, uint32_t n_cols, uint32_t n_rows)
{
	for (uint32_t i = 0; i < n_rows; ++i) {
		for (uint32_t n = 0; n < n_cols; ++n)
			printf("%.3e ", m[i * n_cols + n]);
		printf("\n");
	}
}

float diff_test(float *buf1, float *buf2, uint32_t sz)
{
	float sum_delta = 0, sum = 0;
	for (uint32_t i = 0; i < sz; ++i) {
//		std::cout << buf1[i] << " " << buf2[i] << "\n";
		sum_delta += fabs(buf1[i] - buf2[i]);
		sum += fabs(buf1[i]) + fabs(buf2[i]);
	}
	return sum_delta / sum;
}

void mult_simple(float *m1, float *m2, float *res,
		uint32_t res_rows, uint32_t res_cols, uint32_t len)
{
	for (uint32_t i = 0; i < res_rows; ++i) {
		for (uint32_t k = 0; k < res_cols; ++k) {
			res[i * res_cols + k] = 0;
			for (uint32_t n = 0; n < len; ++n)
				res[i * res_cols + k] += m1[i * len + n]
						* m2[n * res_cols + k];
		}
	}
}

void mult_byrows(float *m1, float *m2, float *res,
		uint32_t res_rows, uint32_t res_cols, uint32_t len)
{
	for (uint32_t i = 0; i < res_rows; ++i) {
		for (uint32_t n = 0; n < res_cols; ++n)
			res[i * res_cols + n] = 0;
		for (uint32_t k = 0; k < len; ++k) {
			for (uint32_t n = 0; n < res_cols; ++n)
				res[i * res_cols + n] += m1[i * len + k]
						* m2[k * res_cols + n];
		}
	}
}

/* -------------------------------------------------------------------------- */

struct float4 {
	float buf[4];
	float &operator[](size_t i) { return buf[i]; }
};

static inline float4 operator+(float4 a, float4 b)
{
	float4 c;
	for (size_t i = 0; i < 4; ++i)
		c[i] = a[i] + b[i];
	return c;
}

static inline float4 operator*(float a, float4 b)
{
	float4 c;
	for (size_t i = 0; i < 4; ++i)
		c[i] = a * b[i];
	return c;
}

static inline
void block_line_eval(float4 *arr1, float4 *arr2, float4 *res, uint32_t len)
{
	float4 tmp[4], tmp2[4];
	for (size_t i = 0; i < 4; ++i)
		tmp[i] = {0, 0, 0, 0};

	for (size_t i = 0; i < 4 * len; i += 4) {
		for (size_t n = 0; n < 4; ++n)
			tmp2[n] = arr2[i+n];
		for (size_t n = 0; n < 4; ++n) {
			for (size_t k = 0; k < 4; ++k)
				tmp[n] = tmp[n] + arr1[i+n][k] * tmp2[k];
		}
	}

	for (size_t i = 0; i < 4; ++i)
		res[i] = tmp[i];
}

void block_reorder1(float4 *in, float4 *out, uint32_t rows, uint32_t cols) // in blocks
{
	for (uint32_t i = 0; i < rows; ++i) {
		for (uint32_t n = 0; n < cols; ++n) {
			float4 *offs = in + n + 4 * cols * i;
			for (uint32_t k = 0; k < 4; ++k)
				*(out++) = offs[k * cols];
		}
	}
}

void block_reorder2(float4 *in, float4 *out, uint32_t rows, uint32_t cols) // in blocks
{
	for (uint32_t n = 0; n < cols; ++n) {
		for (uint32_t i = 0; i < rows; ++i) {
			float4 *offs = in + n + 4 * cols * i;
			for (uint32_t k = 0; k < 4; ++k)
				*(out++) = offs[k * cols];
		}
	}
}

void block_reorder3(float4 *in, float4 *out, uint32_t rows, uint32_t cols)
{
	for (uint32_t n = 0; n < cols; ++n) {
		for (uint32_t i = 0; i < rows; ++i) {
			float4 *offs = out + n + 4 * cols * i;
			for (uint32_t k = 0; k < 4; ++k)
				offs[k * cols] = *(in++);
		}
	}
}

void block_mat_mult(float *m1, float *m2, float *res,
		uint32_t res_rows, uint32_t res_cols, uint32_t len,
		float *tmp1, float *tmp2, float *tmp3)
{
	assert((res_rows % 4 == 0) && (res_cols % 4 == 0) && (len % 4 == 0));
	res_rows /= 4; res_cols /= 4; len /= 4;

	block_reorder1((float4*) m1, (float4*) tmp1, res_rows, len);
	block_reorder2((float4*) m2, (float4*) tmp2, len, res_cols);

	for (uint32_t i = 0; i < res_rows; ++i) {
		for (uint32_t n = 0; n < res_cols; ++n) {
			block_line_eval((float4*) (tmp1 + 16 * len * i),
					(float4*) (tmp2 + 16 * len * n),
					(float4*) (tmp3 + 16 * (i + n * res_rows)),
					len);
		}
	}

	block_reorder3((float4*) tmp3, (float4*) res, res_rows, res_cols);
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
	uint32_t m_sz = 512;
	float *m1 	 = gen_random_mat(m_sz, m_sz);
	float *m2 	 = gen_random_mat(m_sz, m_sz);
	float *tmp_m1 	 = alloc_mat(m_sz, m_sz);
	float *tmp_m2 	 = alloc_mat(m_sz, m_sz);
	float *res_orig  = alloc_mat(m_sz, m_sz);
	float *res_byrow = alloc_mat(m_sz, m_sz);
	float *res_block = alloc_mat(m_sz, m_sz);
	float *tmp_res   = alloc_mat(m_sz, m_sz);

	MEASURE_TIME(mult_simple(m1, m2, res_orig, m_sz, m_sz, m_sz), "simple");
	MEASURE_TIME(mult_byrows(m1, m2, res_byrow, m_sz, m_sz, m_sz), "byrows");
	MEASURE_TIME(block_mat_mult(m1, m2, res_block, m_sz, m_sz, m_sz,
				tmp_m1, tmp_m2, tmp_res), "block");

	std::cout << "simple-byrows diff = "
		<< diff_test(res_orig, res_byrow, m_sz * m_sz) << "\n";

	std::cout << "simple-block diff = "
		<< diff_test(res_orig, res_block, m_sz * m_sz) << "\n";

#if 0
	mat_print(res_orig, m_sz, m_sz);
	printf("\n");
	mat_print(res_block, m_sz, m_sz);
#endif
	return 0;
}
