#include "mem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdbool.h>
#include <time.h>

static mem_region* mem_region_new(void* base, size_t len) {
	mem_region* rg = calloc(1, sizeof(mem_region));
	rg->base = base; rg->len = len; return rg; }

static void mem_regions_add
(mem_regions* rgs, mem_region* rg) {
	if (rgs->root == NULL)
		rgs->root = rgs->head = rg;
	else {
		rgs->head->next = rg;
		rgs->head = rg; }

	rgs->len += rg->len; }

/////////////////////////////////////////////////

const DWORD READONLY =
	PAGE_READONLY | PAGE_EXECUTE | PAGE_EXECUTE_READ |
	PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY;

mem_map mem_map_enum() {
	MEMORY_BASIC_INFORMATION info;
	mem_region* rg; mem_regions* rgs;
	mem_map map = {};

	for (
		const void* head = NULL;
		VirtualQuery(head, &info, sizeof(info)) != 0;
		head += info.RegionSize)
	{
		rg = mem_region_new(info.BaseAddress, info.RegionSize);
		const bool prot =
			(info.State & MEM_COMMIT) == 0 ||
			info.Protect == 0 ||
			info.Protect == PAGE_NOACCESS ||
			(info.Protect & PAGE_GUARD);

		if (prot) rgs = &map.prot;
		else rgs = (info.Protect & READONLY)
		    ? &map.ro : &map.rw;

		mem_regions_add(rgs, rg); }

	return map; }

void mem_rand_init() { time_t ts; time(&ts); srand(ts); }

/////////////////////////////////////////////////

static size_t bit_width(size_t val) {
	size_t bits = 0;
	while (val > 0) { bits++; val >>= 1; }
	return bits; }

static uintptr_t random() {
	const size_t wh0 = bit_width(RAND_MAX),
	             wh = bit_width(UINTPTR_MAX);

	uintptr_t val = 0;
	for (size_t i = 0; i + wh0 < wh; i += wh0)
		val |= (uintptr_t)rand() << i;

	return val; }

void* mem_regions_rand(const mem_regions* rgs) {
	const size_t idx = random() % rgs->len;
	const mem_region* rg = rgs->root;

	for (
		size_t i = 0;
		rg != NULL && i + rg->len < idx;
		i += rg->len, rg = rg->next) {}

	return rg->base + (random() % rg->len); }

void mem_regions_free(mem_regions* rgs) {
	for (
		mem_region *rg0 = NULL, *rg = rgs->root;
		rg != NULL; rg0 = rg, rg = rg->next
	) free(rg0); }
