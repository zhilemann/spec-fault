#define SPEC_PFAULT_INTERNAL

#include <stdlib.h>
#include <stdio.h>
#include "core.h"

size_t TEST_ITERS = 100;

void spec_fetch(uint64_t* mem) {
	register uintptr_t tmp;
	stack_flush();

	asm volatile (
		"call 1f;"
		"mov (%1), %0;"

		"1:"
		"lea 2f(%%rip), %0;"
		"xchg %0, (%%rsp);"
		"ret;"

		"2:"
		
		: "=&r"(tmp) : "r"(mem)); }

void blob_flush(void* mem, void* _) {
	cache_flush(mem); }

void blob_flush_prefetch(void* mem, void* _) {
	cache_flush(mem); spec_fetch(mem); }

int main() {
	volatile blob_t* blob = malloc(sizeof(blob_t));

	printf("running %llu iters...\n", TEST_ITERS);
	printf(
		"regular reads: %llucycles/iter\n",
		microbench(microbench_noop, blob, NULL));

	printf(
		"flush+read: %llucycles/iter\n",
		microbench(blob_flush, blob, NULL));

	printf(
		"flush+spec_fetch+read: %llucycles/iter\n",
		microbench(blob_flush_prefetch, blob, NULL));

	free((void*)blob); return 0; }
