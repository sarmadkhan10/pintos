#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "threads/thread.h"
#include <hash.h>
#include "filesys/off_t.h"

/* page location */
enum page_loc
  {
    FRAME,
    SWAP,
    FILE_SYS,
    ZEROED
  };

/* page error codes */
#define    ACCESS_VIOLATION   -1
#define    MEM_ALLOC_FAIL     -2
#define    PAGE_NOT_FOUND     -3

/* stack size max limit: 8MB */
#define STACK_SIZE_MAX 0x00800000

/* starting user virtual address */
#define START_UVADDR 0x08048000

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
    struct file *file;        /* executable */
    off_t ofs;                /* offset in file */
    uint32_t read_bytes;      /* no. of bytes in page to be read from exec */
    uint32_t zero_bytes;      /* remaining bytes which will be zeroed out */
  };

void                          spt_init_supp_page_table (struct supp_page_table *);
void                          spt_delete_supp_page_table (struct supp_page_table *);
bool                          spt_set_page (struct supp_page_table *, void *, bool );
struct supp_page_table_entry  *spt_find_page (struct supp_page_table *, void *);
int                           vm_load_page (struct supp_page_table *, uint32_t *, void *, bool );
bool                          spt_add_page (struct supp_page_table *, void *, bool ,
                                            struct file *, off_t , uint32_t , uint32_t , enum page_loc );
bool
vm_spt_has_entry (struct supp_page_table *supt, void *page);
bool
vm_spt_mm_unmap(
        struct supp_page_table *supt, uint32_t *pagedir,
        void *page, struct file *f, off_t offset);

#endif /* VM_PAGE_H */
