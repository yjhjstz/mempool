### nginx like memory pool

1.mp_create_pool: create memory pool
2.mp_reset_pool: reset memory pool for reuse;
3.mp_destroy_pool: release the pool memory when all is over;


after the pool is out of its life scope, reset it for resuse;


#### nginx memory pool mode:

where create the memory pool, set the memory pool size is its node size;

* when alloc memory size < node size; alloc it from one of the pool node;
   when all the node doesn't have enough memory for alloc, alloc new node;


* when alloc memory size >= node size; alloc large chunk for it,  likely using malloc directly;


* free: the memory pool won't release the alloc size < node size block until destroy the pool;
        large chunk which size > node size, can be released using mp_free, likely free;


* reuse: mp_reset_pool can release all the large chunk in the pool, 
 but don't release all the allocated node, just reset them for reuse;


#### advantages

* reduce the malloc times to promote performance;

* an reset memory pool can be used for same type work, as web connection;

* using free_mem_pool_list and used_mem_pool_list to promote mem_pool reuse;

#### disadvantages

* linear scan to free larger memory