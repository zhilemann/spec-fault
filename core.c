#include "core.h"

uint64_t timestamp() {
	uint32_t lo, hi;
	asm volatile (
		"rdtscp;"
		"mov %%eax, %0;"
		"mov %%edx, %1;"
		: "=r"(lo), "=r"(hi)
		: : "%eax", "%ecx", "%edx");

	return (uint64_t)hi << 32 | lo; }

/* avoid my reads getting optimized away */
void mem_read(uint64_t* mem) {
	asm volatile (
		"mov (%0), %%rcx;"
		"mfence;"
		: : "r" (mem) : "%rcx"); }

uint64_t microbench(
	void (*func)(void* mem),
	volatile blob_t* blob)
{
	uint64_t ts0, sum = 0;
	for (uint32_t i = 0; i < BENCH_ITERS; i++) {
		func((void*)blob);
		ts0 = timestamp();
		mem_read((uint64_t*)blob);
		sum += timestamp() - ts0; }

	return sum / BENCH_ITERS; }

/////////////////////////////////////////////////

void cache_flush(void* mem) {
	asm volatile (
		"clflush (%0);"
		"mfence;"
		: : "r"(mem)); }

void spec_fetch(void* mem) {
	void* rsp;
	asm volatile ("mov %%rsp, %0;" : "=r"(rsp));
	cache_flush(rsp);
	
	asm volatile (
		"call 1f;"
		"mov (%0), %%rax;"
		"mfence;"

/*		"1:"
		"add $8, %%rsp;"
		"jmp 2f;" */

		"1:"
		"lea 2f(%%rip), %%rax;"
		"xchg %%rax, (%%rsp);"
		"ret;"

		"2:" : : "r"(mem) : "%rax"); }
