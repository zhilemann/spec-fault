#if !defined(CORE_H)
#define CORE_H

#include <stdint.h>

extern size_t BENCH_ITERS;

/* at least a cacheline worth of data */
typedef struct { uint64_t xs[8]; } blob_t;

uint64_t microbench(
    void (*func)(void* mem),
    volatile blob_t* blob);

void cache_flush(void* mem);
void spec_fetch(void* mem);

#endif
