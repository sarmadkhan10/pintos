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

struct supp_page_table
  {
    struct hash spt;
  };

struct supp_page_table_entry
  {
    struct hash_elem elem;
    void *uaddr;              /* pointer to data that goes in the page (virtual addr of the page) */
    enum page_loc loc;
    bool writable;           /* true if write allowed. otherwise read-only */
  };

void    spt_create_supp_page_table (struct supp_page_table *);
void    spt_delete_supp_page_table (struct supp_page_table *);
bool    spt_set_page (struct supp_page_table *, void *, bool );

#endif /* VM_PAGE_H */
