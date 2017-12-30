#include "threads/thread.h"
#include "vm/page.h"
#include <hash.h>
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <string.h>
#include "threads/palloc.h"
#include "vm/frame.h"

static unsigned spt_hash_func(const struct hash_elem *elem, void *aux);
static bool     spt_less_hash_func(const struct hash_elem *a, const struct hash_elem *b, void *aux);

/* allocate and initialize supplemental page table spt */
void
spt_create_supp_page_table (struct supp_page_table *spt)
{
  spt = malloc (sizeof (struct supp_page_table));
  hash_init (&spt->spt, spt_hash_func, spt_less_hash_func, NULL);
}

/* free supplemental page table spt resources */
void
spt_delete_supp_page_table (struct supp_page_table *spt)
{
  free (spt);
}

/* add the page entry in spt if not already present in spt. Returns true in that case.
   Otherwise returns false without inserting it.*/
bool
spt_set_page (struct supp_page_table *spt, void *uaddr, bool writable)
{
  /* create a new spt entry add the page address in spt */
  struct supp_page_table_entry *spt_entry = malloc (sizeof (struct supp_page_table_entry));
  spt_entry->uaddr = uaddr;
  spt_entry->loc = FRAME;
  spt_entry->writable = writable;

  bool inserted = (hash_insert (&spt->spt, &spt_entry->elem) == NULL) ? true : false;

  return inserted;
}

/* find the page in spt given the starting address of the page */
struct supp_page_table_entry *
spt_find_page (struct supp_page_table *spt, void *paddr)
{
  struct supp_page_table_entry spt_e;
  spt_e.uaddr = paddr;
  struct hash_elem *e;

  e = hash_find (&spt->spt, &spt_e.elem);
  return (e != NULL) ? hash_entry (e, struct supp_page_table_entry, elem) : NULL;
}

/* spt hash function for hash */
static unsigned
spt_hash_func(const struct hash_elem *elem, void *aux UNUSED)
{
  struct supp_page_table_entry *entry = hash_entry (elem, struct supp_page_table_entry, elem);
  return hash_bytes (&entry->uaddr, sizeof (entry->uaddr));
}

/* spt less hash function */
static bool
spt_less_hash_func(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct supp_page_table_entry *a_entry = hash_entry (a, struct supp_page_table_entry, elem);
  struct supp_page_table_entry *b_entry = hash_entry (b, struct supp_page_table_entry, elem);
  return a_entry->uaddr < b_entry->uaddr;
}

/* load a page paddr in mem. paddr must reside in in user vm. returns true if loaded
   successfully. Returns false if out of memory or access violated.
   the write parameter is used for access control */
int
vm_load_page (struct supp_page_table *spt, uint32_t *pagedir, void *paddr, bool write)
{
  int error = true;

  struct supp_page_table_entry *spt_e = spt_find_page (spt, paddr);

  if (spt_e != NULL)
    {
      /* if the process was trying to write to a read-only page, kill it */
      if (!spt_e->writable && write)
          error = ACCESS_VIOLATION;
      else
        {
          /* allocation a frame to store the page */
          void *frame = vm_frame_allocate (PAL_USER);

          if (frame == NULL)
              error = MEM_ALLOC_FAIL;
          else
            {
              switch (spt_e->loc)
              {
                case FRAME:
                  /* the page is already present in memory. Continue */
                  break;
                case SWAP:
                  /* TODO */
                  PANIC ("SWAP not implemented yet");
                  break;
                case FILE_SYS:
                  /* TODO */
                  PANIC ("Loading from filesys not implemented yet");
                  break;
                case ZEROED:
                  memset (frame, 0, PGSIZE);
                  break;
                default:
                  PANIC ("vm_load_page: should not reach here.");
              }

              if (!pagedir_set_page (pagedir, paddr, frame, spt_e->writable))
                {
                  error = MEM_ALLOC_FAIL;
                  vm_frame_free (frame);
                }
              else
                {
                  spt_e->loc = FRAME;
                  pagedir_set_dirty (pagedir, frame, false);
                }
            }
        }
    }
  else
    {
      /* page not found in spt */
      error = PAGE_NOT_FOUND;
    }

  return error;
}
