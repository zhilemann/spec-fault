// a little benchmark for `spec-fault`

#include "../lib/spec-fault.h"
#include "mem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>
#include <stdio.h>

typedef struct { bool read, write; } test_faults;

/* make sure the page is present in physical mem */
void test_init(const void* mem) {
	register uintptr_t tmp;
	asm volatile (
		"mov (%1), %0; clflush (%1); mfence;"
		: "=&r"(tmp) : "r"(mem)); }

test_faults test_fault(void* mem) {
	const bool read = spec_fault_read(mem),
	           write = spec_fault_write(mem);

	return (test_faults) { read, write }; }

/////////////////////////////////////////////////

typedef struct { size_t neg, pos; } test_run0;
typedef struct { test_run0 read, write; } test_run;

test_run test_rand_run(const mem_map* map) {
	void *ro = mem_regions_rand(&map->ro),
	     *rw = mem_regions_rand(&map->rw),
	     *prot = mem_regions_rand(&map->prot);

	test_init(ro); test_init(rw);
	const test_faults
		ro1 = test_fault(ro), rw1 = test_fault(rw),
		prot1 = test_fault(prot);

	test_run run = {};
	if (!ro1.read) run.read.neg++;
	if (ro1.write) run.write.pos++;

	if (!rw1.read) run.read.neg++;
	if (!rw1.write) run.write.neg++;

	if (prot1.read) run.read.pos++;
	if (prot1.write) run.write.pos++;

	return run; }

/////////////////////////////////////////////////

void test_run_add(test_run* run, const test_run* run0) {
	run->read.neg += run0->read.neg;
	run->read.pos += run0->read.pos;

	run->write.neg += run0->write.neg;
	run->write.pos += run0->write.pos; }

int main() {
	mem_map map; mem_rand_init();
	size_t runs = 0;
	test_run run = {}, run0;

	while (true) {
		map = mem_map_enum();
		for (size_t i = 0; i < 256; i++) {
			for (size_t j = 0; j < 16; j++, runs++) {
				run0 = test_rand_run(&map);
				test_run_add(&run, &run0); }
		
			printf("\33[2K\33[A\33[2K\r");

			printf("[+] errors over %llu test runs:", runs);
			printf(
				"[+] reads: neg=%llu, pos=%llu\n",
				run.read.neg, run.read.pos);

			printf(
				"[+] writes: neg=%llu, pos=%llu",
				run.write.neg, run.write.pos);

			Sleep(0); }

		mem_regions_free(&map.ro);
		mem_regions_free(&map.rw);
		mem_regions_free(&map.prot); }

	return 0; }
