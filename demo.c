// a "anti-debug" demo for cool GIFs

#include "lib/spec-fault.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

static void mem_read(void* mem) {
	asm volatile ("clflush (%0); mfence;" : : "r"(mem)); }

static char* alloc_secret() {
	char* mem = VirtualAlloc(
		NULL, 4096, MEM_COMMIT, PAGE_READWRITE);

	strcat(mem, "super secret data!!!");
	return mem; }

int main() {
	char* mem = alloc_secret(); mem_read(mem);
	printf("[+] watching 0x%llx...\n", (uintptr_t)mem);
	
	while (true) {
		const bool read = spec_fault_read(mem),
		           write = spec_fault_write(mem);
		
		printf(
			"\33[2K\r[+] read BP: %i, write BP: %i",
			read, write);

		if (!read) mem_read(mem); Sleep(250); }
	
	return 0; }
