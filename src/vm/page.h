#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "threads/thread.h"
#include <hash.h>

/* page location */
enum page_loc
  {
    FRAME,
    SWAP,
    FILE_SYS,
    ZEROED
  };

/* page error codes */
#define    ACCESS_VIOLATION -1
#define    MEM_ALLOC_FAIL  -2
#define    PAGE_NOT_FOUND   -3

/* stack size max limit: 8MB */
#define STACK_SIZE_MAX 0x00800000

struct supp_page_table
  {
    struct hash spt;
  };

struct supp_page_table_entry
  {
    struct hash_elem elem;
    void *uaddr;              /* pointer to data that goes in the page (virtual addr of the page) */
    enum page_loc loc;
    bool writable;            /* true if write allowed. otherwise read-only */
    size_t swap_index;        /* if the page is in swap, this is the index in swap bitmap */
  };

void                          spt_init_supp_page_table (struct supp_page_table *);
void                          spt_delete_supp_page_table (struct supp_page_table *);
bool                          spt_set_page (struct supp_page_table *, void *, bool );
struct supp_page_table_entry  *spt_find_page (struct supp_page_table *, void *);
int                           vm_load_page (struct supp_page_table *, uint32_t *, void *, bool );

#endif /* VM_PAGE_H */
