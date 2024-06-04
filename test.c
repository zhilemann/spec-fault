// a little benchmark for `spec-pfault`

#include "src/spec-pfault.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/////////////////////////////////////////////////

typedef struct _mem_region {
	struct _mem_region* next;
	const void* base; size_t len; } mem_region;

typedef struct {
	mem_region *root, *head;
	size_t len; } mem_regions;

mem_region* mem_region_new(const void* base, size_t len) {
	mem_region* rg = calloc(1, sizeof(mem_region));
	rg->base = base; rg->len = len; return rg; }

void mem_regions_add(mem_regions* rgs, mem_region* rg) {
	if (rgs->root == NULL)
		rgs->root = rgs->head = rg;
	else {
		rgs->head->next = rg;
		rgs->head = rg; }

	rgs->len += rg->len; }

typedef struct { mem_regions reg, flt; } mem_map;

mem_map mem_map_enum() {
	printf("[+] enumerating memory regions...\n");

	MEMORY_BASIC_INFORMATION info;
	mem_map map = {}; mem_region* rg;

	for (
		const void* head = NULL;
		VirtualQuery(head, &info, sizeof(info)) != 0;
		head += info.RegionSize)
	{
		rg = mem_region_new(info.BaseAddress, info.RegionSize);
		const bool flt =
			info.Protect == 0 ||
			info.Protect == PAGE_NOACCESS ||
			(info.Protect & PAGE_GUARD) != 0;

		printf(
			"> %s base=%llx len=%llu\n",
			!flt ? "regular" : "faulty",
			(uintptr_t)rg->base, rg->len);

		mem_regions_add(!flt ? &map.reg : &map.flt, rg); }

	printf("\n"); return map; }

/////////////////////////////////////////////////

typedef enum {
	TEST_OK = 0, TEST_FALSE_NEG, TEST_FALSE_POS,
	TEST_BROKEN = TEST_FALSE_NEG | TEST_FALSE_POS
} test_run;

size_t bit_width(size_t val) {
	size_t bits = 0;
	while (val > 0) { bits++; val >>= 1; }
	return bits; }

void random_init() { time_t ts; time(&ts); srand(ts); }
uintptr_t random() {
	const size_t wh0 = bit_width(RAND_MAX),
	             wh = bit_width(UINTPTR_MAX);

	uintptr_t val = 0;
	for (size_t i = 0; i + wh0 < wh; i += wh0)
		val |= (uintptr_t)rand() << i;

	return val; }

const void* mem_regions_rand(const mem_regions* rgs) {
	const size_t idx = random() % rgs->len;
	const mem_region* rg = rgs->root;

	for (
		size_t i = 0;
		rg != NULL && i + rg->len < idx;
		i += rg->len, rg = rg->next) {}

	return rg->base + (random() % rg->len); }

/* make sure the page is present in physical mem */
void mem_init(const void* mem) {
	register uintptr_t tmp;
	asm volatile (
		"mov (%1), %0; clflush (%1); mfence;"
		: "=&r"(tmp) : "r"(mem)); }

test_run test(const mem_map* map) {
	const void *reg = mem_regions_rand(&map->reg),
	           *flt = mem_regions_rand(&map->flt);

	printf("[+] let regular addr = 0x%llx\n", (uintptr_t)reg);
	printf("[+] let faulty addr = 0x%llx\n", (uintptr_t)flt);

	mem_init(reg);
	const bool reg1 = spec_pfault(reg),
	           flt1 = spec_pfault(flt);
	
	printf(
		"> spec_pfault(0x%llx) = %i: %s\n",
		(uintptr_t)reg, reg1, reg1 ? "FAIL" : "OK");

	printf(
		"> spec_pfault(0x%llx) = %i: %s\n\n",
		(uintptr_t)flt, flt1, flt1 ? "OK" : "FAIL");

	if (reg1) return flt1
		? TEST_FALSE_POS : TEST_BROKEN;
	else return flt1
		? TEST_OK : TEST_FALSE_NEG; }

/////////////////////////////////////////////////

int main() {
	random_init();
	mem_map map = mem_map_enum();

	size_t false_neg = 0, false_pos = 0;
	for (size_t i = 0; i < 64; i++) {
		for (size_t j = 0; j < 4; j++) {
			test_run run = test(&map);
			if (run & TEST_FALSE_NEG) false_neg++;
			if (run & TEST_FALSE_POS) false_pos++; }

		Sleep(1); }

	printf(
		"256 runs: %llu false neg, %llu false pos\n",
		false_neg, false_pos);

	return 0; }
