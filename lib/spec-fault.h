#if !defined(SPEC_FAULT_H)
#define SPEC_FAULT_H

#include <stdbool.h>

#if defined(SPEC_FAULT_C)
	#include <stdint.h>

	#define align_to(n) _Alignas(n)
	
	/* a cache line worth of data */
	typedef struct { uint8_t _[64]; } blob;
	typedef struct {
		blob test, _pad, junk;
	} blobset;

	typedef void (*spec_test_fn)(void* mem, blobset* blobset);
	typedef struct { uint64_t reg, spec; } spec_stats;
	
	/* speculate & buy some time */
	#define spec_call(asm, tmp, junk) \
		"call 1f;" asm \
		"1: vmovups 48(" junk "), %%ymm0;" \
		"vmovups 32(" junk "), %%ymm1;" \
		"vmovups 16(" junk "), %%ymm2;" \
		"vmovups (" junk "), %%ymm3;" \
		"vsqrtpd %%ymm0, %%ymm0;" \
		"vaddps %%ymm0, %%ymm1, %%ymm2;" \
		"vsqrtpd %%ymm1, %%ymm1;" \
		"vaddps %%ymm1, %%ymm2, %%ymm3;" \
		"vsqrtpd %%ymm2, %%ymm2;" \
		"vaddps %%ymm2, %%ymm3, %%ymm0;" \
		"vsqrtpd %%ymm3, %%ymm3;" \
		"vaddps %%ymm3, %%ymm0, %%ymm1;" \
		"lea 2f(%%rip), " tmp ";" \
		"xchg " tmp ", (%%rsp);" \
		"ret; 2:"
#endif

bool spec_fault_read(const void* mem);
bool spec_fault_write(void* mem);

#endif
