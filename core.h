#if !defined(CORE_H)
#define CORE_H

#include <stdbool.h>
#include <stdint.h>

#define align_to(n) _Alignas(n)

extern size_t TEST_ITERS;
bool check_pagefault(void* mem);

#if defined(SPEC_PFAULT_INTERNAL)
	/* at least a cacheline worth of data */
	typedef struct { uint64_t xs[8]; } blob_t;

	uint64_t microbench(
		void (*func)(void* mem, void* data),
		volatile void* mem, void* data);

	void microbench_noop(void* _1, void* _2);

	void cache_flush(void* mem);
	void stack_flush();
#endif

#endif
