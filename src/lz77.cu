#include <iostream>
#include <iomanip>
#include <cuda.h>
#include <cstdint>
#include <vector>
#include <cstdio>
#include <cassert>
#include "aux.h"
#include <thrust/sort.h>
#include <thrust/functional.h>
#include <thrust/copy.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/random.h>
#include <thrust/inner_product.h>
#include <thrust/binary_search.h>
#include <thrust/adjacent_difference.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>

#define MATCH_LENGTH 1024

#define handle_error(ans) { cuda_assert((ans), __FILE__, __LINE__); }
inline void cuda_assert(cudaError_t code, const char *file, int line, bool abort=true)
{
	if (code != cudaSuccess) 
	{
		fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

void print_vector(const std::string &name, const thrust::device_vector<uint32_t> &v)
{
	std::cout << "  " << std::setw(20) << name << "  ";
	thrust::copy(v.begin(), v.end(), std::ostream_iterator<uint32_t>(std::cout, " "));
	std::cout << std::endl;
}

/*__global__ void lz77_find_matches(uint8_t *data, uint32_t size, uint32_t **matches, uint32_t *num_matches)
{
	uint32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	uint32_t match_idx;
	if (idx < size - 2)
	{
		match_idx = *((uint32_t*) (data + idx)) & 0x00FFFFFF;
		if (*(matches[match_idx]) == 0)
		{
			atomicAdd(num_matches, 1);
			matches[match_idx] = (uint32_t*) malloc(MATCH_LENGTH * sizeof(uint32_t));
			matches[match_idx][0] = 1;
			matches[match_idx][1] = idx;
		}
		else
		{
			matches[match_idx][0]++;
			matches[match_idx][matches[match_idx][0]] = idx;
		}
	}
}*/

__global__ void lz77_stage1(uint8_t *data, uint32_t size, uint32_t *hashes, uint32_t *idx_list) //size is two less than sizeof(data)
{
	const uint32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < size)
	{
		hashes[idx] = *((uint32_t*) (data + idx)) & 0x00FFFFFF;
		idx_list[idx] = idx;
	}
}

lz77_data* lz77_cuda(std::vector<uint8_t> *data)
{
	uint32_t block_size = 1024;
	uint32_t grid_size = data -> size() / block_size;
	uint32_t data_size = data -> size() - 2;
	uint8_t *device_file = NULL;
	uint32_t *device_idx_list = NULL;
	uint32_t *device_hashes = NULL;
	//thrust::device_vector<uint32_t> device_hashes_vector(data_size);
	//uint32_t *device_hashes = device_hashes_vector.data();
	if (data -> size() % block_size) { grid_size++; }
	handle_error(cudaMalloc(&device_file, data -> size()));
	handle_error(cudaMalloc(&device_idx_list, data_size * sizeof(uint32_t)));
	handle_error(cudaMalloc(&device_hashes, data_size * sizeof(uint32_t)));
	assert(device_file != NULL);
	assert(device_idx_list != NULL);
	assert(device_hashes != NULL);
	handle_error(cudaMemcpy(device_file, data -> data(), data -> size(), cudaMemcpyHostToDevice));
	lz77_stage1<<<grid_size, block_size>>>(device_file, data -> size() - 2, device_hashes, device_idx_list);
	handle_error(cudaDeviceSynchronize());
	//thrust::stable_sort_by_key(device_hashes, device_hashes + data_size, device_idx_list); 
	thrust::device_vector<uint32_t> histogram_input;
	thrust::device_vector<uint32_t> histogram_values;
	thrust::device_vector<uint32_t> histogram_counts;
	thrust::device_ptr<uint32_t> hash_ptr(device_hashes);
	thrust::copy(device_hashes, device_hashes + data_size, histogram_input.begin());
	//thrust::copy(device_hashes_vector.begin(), device_hashes_vector.end(), histogram_data.begin();
	thrust::sort(histogram_input.begin(), histogram_input.end());
	uint32_t unique_keys = thrust::inner_product(histogram_input.begin(), histogram_input.end() - 1, histogram_input.begin() + 1, (uint32_t) 1, thrust::plus<uint32_t>(), thrust::not_equal_to<uint32_t>());
	histogram_values.resize(unique_keys);
	histogram_counts.resize(unique_keys);
	thrust::reduce_by_key(histogram_input.begin(), histogram_input.end(), thrust::constant_iterator<uint32_t>(1), histogram_values.begin(), histogram_counts.begin());
	print_vector("histogram values", histogram_values);
	print_vector("histogram counts", histogram_counts);
	return NULL;
}
