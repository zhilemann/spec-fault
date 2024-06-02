#define SPEC_PFAULT_INTERNAL

#include <stddef.h>
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
	register uintptr_t tmp;
	asm volatile (
		"mov (%1), %0;"
		"mfence;"

		: "=&r"(tmp) : "r" (mem)); }

uint64_t microbench(
	void (*func)(void* mem, void* data),
	volatile void* mem, void* data)
{
	uint64_t ts0, sum = 0;
	for (uint32_t i = 0; i < TEST_ITERS; i++) {
		func((void*)mem, data);
		ts0 = timestamp();

		mem_read((uint64_t*)mem);
		sum += timestamp() - ts0; }

	return sum / TEST_ITERS; }

void microbench_noop(void* _1, void* _2) {}

/////////////////////////////////////////////////

void cache_flush(void* mem) {
	asm volatile (
		"clflush (%0);"
		"mfence;"

		: : "r"(mem)); }

void stack_flush() {
	asm volatile (
		"clflush (%rsp);"
		"mfence;"); }

void spec_test(uint64_t* src, uint64_t* dst) {
	register uintptr_t tmp;
	stack_flush();
	
	asm volatile (
		"call 1f;"
		"mov (%1), %0;"
		"test $-1, %0;"
		"cmovz %2, %0;"
		"cmovnz %2, %0;"
		"mov (%0), %0;"

		"1:"
		"lea 2f(%%rip), %0;"
		"xchg %0, (%%rsp);"
		"ret;"

		"2:"
		
		: "=&r"(tmp) : "r"(src), "r"(dst));
	asm volatile (
		"call 1f;"
		"mov (%1), %0;"
		"test $-1, %0;"
		"cmovz %2, %0;"
		"cmovnz %2, %0;"
		"mov (%0), %0;"

		"1:"
		"lea 2f(%%rip), %0;"
		"xchg %0, (%%rsp);"
		"ret;"

		"2:"
		
		: "=&r"(tmp) : "r"(src), "r"(dst)); }

void try_spec_test(void* mem, void* data) {
	cache_flush(mem);
	spec_test((uint64_t*)data, mem); }

bool check_pagefault(void* mem) {
	volatile blob_t blob[2];

	const uint64_t regular =
		microbench(microbench_noop, &blob[1], NULL);

	const uint64_t spec_test =
		microbench(try_spec_test, &blob[1], mem);

	return spec_test > 2 * regular; }
