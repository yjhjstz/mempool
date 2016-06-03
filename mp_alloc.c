#include "mp_alloc.h"

static void *mp_alloc_block(mp_pool_t *pool, size_t size);
static void *mp_alloc_large(mp_pool_t *pool, size_t size);

mp_pool_t * mp_create_pool(size_t size)
{
    int ret;
    mp_pool_t  *p;

    ret = posix_memalign(&p, MP_ALIGNMENT, size + sizeof(mp_pool_t) + sizeof(mp_node_t));
    if (ret) {
        return NULL;
    }

    p->max = (size < MP_MAX_ALLOC_FROM_POOL) ? size : MP_MAX_ALLOC_FROM_POOL;

    p->current = p->head;
    p->large = NULL;

    p->head->last = (unsigned char *) p + sizeof(mp_pool_t) + sizeof(mp_node_t);
    p->head->end = p->head->last + size;
    p->head->next = NULL;
    p->head->failed = 0;

    return p;
}


void mp_destroy_pool(mp_pool_t *pool)
{
    mp_node_t     *h, *n;
    mp_large_t    *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }

        /* as mp_pool_large_t l has been allocated from the pool, whithout release here */
    }

    // release other node
    h = pool->head->next;

    while (h) {
        n = h->next;
        free(h);
        h = n;
    }

    // release the pool
    free(pool);
}


void mp_reset_pool(mp_pool_t *pool)
{
    mp_node_t   *h;
    mp_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    pool->large = NULL;

    for (h = pool->head; h; h = h->next) {
        h->last = (unsigned char *) h + sizeof(mp_node_t);
    }

}


void * mp_alloc(mp_pool_t *pool, size_t size)
{
    unsigned char     *m;
    mp_node_t  *p;

    if (size <= pool->max) {

        p = pool->current;

        do {
            m = mp_align_ptr(p->last, MP_ALIGNMENT);

            if ((size_t) (p->end - m) >= size) {
                p->last = m + size;

                return m;
            }

            p = p->next;

        } while (p);

        return mp_alloc_block(pool, size);
    }

    return mp_alloc_large(pool, size);
}


void * mp_nalloc(mp_pool_t *pool, size_t size)
{
    unsigned char     *m;
    mp_node_t  *p;

    if (size <= pool->max) {

        p = pool->current;

        do {
            m = p->last;

            if ((size_t) (p->end - m) >= size) {
                p->last = m + size;

                return m;
            }

            p = p->next;

        } while (p);

        return mp_alloc_block(pool, size);
    }

    return mp_alloc_large(pool, size);
}


static void * mp_alloc_block(mp_pool_t *pool, size_t size)
{
    unsigned char      *m;
    int ret;
    size_t      psize;
    mp_node_t   *h, *p, *new, *current;

    h = pool->head;
    psize = (size_t) (h->end - (unsigned char *) h);

    ret = posix_memalign(&m, MP_ALIGNMENT, psize);
    if (ret) {
        return NULL;
    }

    new = (mp_node_t *) m;
    
    new->end = m + psize;
    new->next = NULL;
    new->failed = 0;

    m += sizeof(mp_node_t);
    m = mp_align_ptr(m, MP_ALIGNMENT);
    new->last = m + size;

    current = pool->current;

    for (p = current; p->next; p = p->next) {
        if (p->failed++ > 4) { //here make the search alloc path short
            current = p->next;
        }
    }

    p->next = new;

    pool->current = current ? current : new;

    return m;
}


static void * mp_alloc_large(mp_pool_t *pool, size_t size)
{
    void              *p;
    size_t             n;
    mp_large_t  *large;

    p = malloc(size);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = mp_alloc(pool, sizeof(mp_large_t));
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


void * mp_memalign(mp_pool_t *pool, size_t size, size_t alignment)
{
    void        *p;
    mp_large_t  *large;
    int ret;
    ret = posix_memalign(&p, alignment, size);
    if (ret) {
        return NULL;
    }

    // without reuse the mp_large_t struct;
    large = mp_alloc(pool, sizeof(mp_large_t));
    if (large == NULL) {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


void mp_free(mp_pool_t *pool, void *p)
{
    mp_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = NULL;

            return;
        }
    }

}


void * mp_calloc(mp_pool_t *pool, size_t size)
{
    void *p;

    p = mp_alloc(pool, size);
    if (p) {
        memset(p, 0, size);
    }

    return p;
}

