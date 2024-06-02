#include <stdio.h>
#include "core.h"

size_t BENCH_ITERS = 100;

void blob_noop(void* mem) {}
void blob_flush_prefetch(void* mem) {
	cache_flush((void*)mem);
	spec_fetch((void*)mem); }

int main() {
	volatile blob_t blob;

	printf("running %llu iters...\n", BENCH_ITERS);
	printf(
		"regular reads: %llucycles/iter\n",
		microbench(blob_noop, &blob));

	printf(
		"flush+read: %llucycles/iter\n",
		microbench(cache_flush, &blob));

	printf(
		"flush+spec_fetch+read: %llucycles/iter\n",
		microbench(blob_flush_prefetch, &blob));

	return 0; }
