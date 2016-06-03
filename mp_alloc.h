#ifndef MP_ALLOC_H_
#define MP_ALLOC_H_

#define _XOPEN_SOURCE 600  
#define _GNU_SOURCE  
#include <stdlib.h>  
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


/*
 * MP_MAX_ALLOC_FROM_POOL should be (PAGE_SIZE - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define PAGE_SIZE  4096
#define MP_MAX_ALLOC_FROM_POOL  (PAGE_SIZE - 1)
//#define MP_DEFAULT_POOL_SIZE    (16 * 1024)
#define MP_DEFAULT_POOL_SIZE    (1 << 14) 

#define MP_ALIGNMENT       16
#define mp_align(n, alignment) (((n) + (alignment - 1)) & ~(alignment - 1))
#define mp_align_ptr(p, alignment) (void *)((((size_t) p) + (alignment - 1)) & ~(alignment - 1))
#define MP_MIN_POOL_SIZE mp_align((sizeof(mp_pool_t) + 2 * sizeof(mp_large_t)), MP_ALIGNMENT)


// forward declaration
typedef struct mp_node_s   mp_node_t;
typedef struct mp_large_s  mp_large_t;

struct mp_large_s {
    mp_large_t     *next;
    void           *alloc;
};

struct mp_node_s {
    unsigned char        *last;
    unsigned char        *end;
    mp_node_t            *next;
    size_t               failed;
};


typedef struct {
    size_t               max;
    mp_node_t           *current;
    mp_large_t          *large;
#ifdef MP_DEBUG_LOG
    int  log;
#endif
    mp_node_t        head[0];
} mp_pool_t;



mp_pool_t *mp_create_pool(size_t size);
void mp_destroy_pool(mp_pool_t *pool);
void mp_reset_pool(mp_pool_t *pool);

void *mp_alloc(mp_pool_t *pool, size_t size);
void *mp_nalloc(mp_pool_t *pool, size_t size);
void *mp_calloc(mp_pool_t *pool, size_t size);

void *mp_memalign(mp_pool_t *pool, size_t size, size_t alignment);
void mp_free(mp_pool_t *pool, void *p);

#endif /* MP_ALLOC_H_ */
