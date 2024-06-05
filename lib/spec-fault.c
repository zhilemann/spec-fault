#define SPEC_FAULT_C
#include "spec-fault.h"

static uint64_t timestamp() {
	uint32_t lo, hi;
	asm volatile (
		"rdtscp;"
		"mov %%eax, %0;"
		"mov %%edx, %1;"

		: "=r"(lo), "=r"(hi)
		: : "%eax", "%ecx", "%edx");

	return (uint64_t)hi << 32 | lo; }

/* avoid my reads getting optimized out */
static void mem_read(void* mem) {
	register uintptr_t tmp;
	asm volatile (
		"mov (%1), %0; mfence;"
		: "=&r"(tmp) : "r" (mem)); }

static uint64_t timed_read(void* mem) {
	uint64_t ts0 = timestamp(); mem_read(mem);
	return timestamp() - ts0; }

static void cache_flush(void* mem) {
	asm volatile ("clflush (%0); mfence;" : : "r"(mem)); }

static void uint64_min(uint64_t* x, uint64_t y) {
	if (*x > y) *x = y; }

static void spec_stats_add
(spec_stats* stats, const spec_stats* stats0) {
	stats->reg += stats0->reg;
	stats->spec += stats0->spec; }

/////////////////////////////////////////////////

/* memory for side-channel to work with */
align_to(64) static blobset BLOBSET = {};

static spec_stats spec_fault0
(void* mem, blobset* blobset, spec_test_fn test_fn) {
	spec_stats stats = { ~0, ~0 };
	void* test = &blobset->test;
	uint64_t ts0;

	for (size_t j = 0; j < 32; j++) {
		cache_flush(test); mem_read(test);
		uint64_min(&stats.reg, timed_read(test)); }

	for (size_t j = 0; j < 32; j++) {
		for (size_t k = 0; k < 4; k++) cache_flush(test);
		for (size_t k = 0; k < 256; k++)
			test_fn(mem, blobset);

		uint64_min(&stats.spec, timed_read(test)); }
	
	return stats; }

static bool spec_fault(spec_test_fn test_fn, void* mem) {
	spec_stats stats = {}, stats0;

	for (size_t i = 0; i < 32; i++) {
		stats0 = spec_fault0(mem, &BLOBSET, test_fn);
		spec_stats_add(&stats, &stats0);

		if (i >= 8) {
			const uint64_t reg = stats.reg;
			if (stats.spec < (reg + reg / 2)) return false;
			if (stats.spec > 4 * reg) return true; } }

	const uint64_t reg = stats.reg;
	return stats.spec > (2 * reg + reg / 2); }

/////////////////////////////////////////////////

/* if `mem` is invalid, then (I suppose)
   `test` fails and `cmov*` are skipped */
void spec_read(void* mem, blobset* blobset) {
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

		"" : "=&r"(tmp1), "=&r"(tmp2)
		: "r"(mem), "r"(&blobset->test),
		  "r"(&blobset->junk)); }

void spec_write(void* mem, blobset* blobset) {
	register uintptr_t tmp1, tmp2;
	asm volatile (
		"prefetchnta (%2);"
		spec_call(
			"movq $-1, (%2);"
			"mov (%2), %1;"
			"cmp $-1, %1;"
			"cmoveq %3, %0;"
			"mov (%0), %1;"
			"jmp 2f;", "%0", "%4")
		
		"" : "=&r"(tmp1), "=&r"(tmp2)
		: "r"(mem), "r"(&blobset->test),
		  "r"(&blobset->junk)); }

bool spec_fault_read(const void* mem) {
	return spec_fault(spec_read, (void*)mem); }

bool spec_fault_write(void* mem) {
	return spec_fault(spec_write, mem); }
