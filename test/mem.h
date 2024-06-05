#if !defined(MEM_H)
#define MEM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct _mem_region {
	struct _mem_region* next;
	void* base; size_t len; } mem_region;

typedef struct {
	mem_region *root, *head;
	size_t len; } mem_regions;

typedef struct {
	mem_regions ro, rw, prot; } mem_map;

mem_map mem_map_enum();
void mem_rand_init();

void* mem_regions_rand(const mem_regions* rgs);
void mem_regions_free(mem_regions* rgs);

#endif
