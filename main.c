#include <stdlib.h>
#include <stdio.h>
#include "core.h"

size_t TEST_ITERS = 500;

int main() {
	for (uint32_t i = 0; i < 50; i++) {
		char* mem1 = malloc(4096);
		char* mem2 = (char*)0x77777;

		printf(
			"RW-: check_pagefault(%llx) = %i\n",
			(uintptr_t)mem1, check_pagefault(mem1));

		printf(
			"---: check_pagefault(%llx) = %i\n",
			(uintptr_t)mem2, check_pagefault(mem2));

		printf("\n"); free(mem1); }

	return 0; }
