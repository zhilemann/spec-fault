// a "anti-debug" demo for cool GIFs

#include "src/spec-pfault.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>

char* alloc_secret() {
	char* mem = VirtualAlloc(
		NULL, 4096, MEM_COMMIT, PAGE_READWRITE);

	strcat(mem, "super secret data!!!");
	asm volatile ("clflush (%0); mfence;" : : "r"(mem));
	
	return mem; }

int main() {
	char* mem = alloc_secret();
	printf("[+] watching 0x%llx...\n", (uintptr_t)mem);
	
	while (true) {
		const bool ok = !spec_pfault(mem);
		const char* msg = ok
			? "no breakpoint detected."
			: "the program is being debugged!";

		printf("\33[2K\r[+] %s", msg);
		Sleep(250); }
	
	return 0; }
