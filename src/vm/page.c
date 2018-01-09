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
spt_init_supp_page_table (struct supp_page_table *spt)
{
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
  ASSERT (spt != NULL);

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
  ASSERT (spt != NULL);

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
  return hash_int ((int) entry->uaddr);
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

              printf ("from vm_load\n");
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
              printf ("after vm_load\n");
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


bool
spt_add_page (struct supp_page_table *spt, void *paddr, bool writable, struct file *file,
                   off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, enum page_loc loc)
{
  bool inserted;

  /* check if an entry for paddr already exists */
  struct supp_page_table_entry *spt_e = spt_find_page (spt, paddr);

  if (spt_e != NULL)
    {
      inserted = false;
    }
  else
    {
      /* create a new spt entry add the page address in spt */
      struct supp_page_table_entry *spt_entry = malloc (sizeof (struct supp_page_table_entry));
      spt_entry->uaddr = paddr;
      spt_entry->writable = writable;
      spt_entry->file = file;
      spt_entry->ofs = ofs;
      spt_entry->read_bytes = read_bytes;
      spt_entry->zero_bytes = zero_bytes;
      spt_entry->loc = loc;

      inserted = (hash_insert (&spt->spt, &spt_entry->elem) == NULL) ? true : false;
    }

  return inserted;
}


/**
 * Install a page (specified by the starting address `upage`)
 * on the supplemental page table, of type FROM_FILESYS.

bool
vm_spt_install_filesys (struct supp_page_table *supt, void *upage,
                         struct file * file, off_t offset, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  struct supp_page_table_entry *spte;
  spte = (struct supp_page_table_entry *) malloc(sizeof(struct supp_page_table_entry));

  spte->uaddr = upage;
  spte->loc = FILE_SYS;
  spte->file = file;
  spte->ofs = offset;
  spte->read_bytes = read_bytes;
  spte->zero_bytes = zero_bytes;
  spte->writable = writable;

  struct hash_elem *prev_elem;
  prev_elem = hash_insert (&supt->spt, &spte->elem);
  if (prev_elem == NULL) return true;

  // TODO there is already an entry.
  PANIC("Duplicated SUPT entry for filesys-page");
  return false;
}*/
struct supp_page_table_entry*
vm_spt_lookup (struct supp_page_table *supt, void *page)
{
    // create a temporary object, just for looking up the hash table.
    struct supp_page_table_entry spte_temp;
    spte_temp.uaddr = page;

    struct hash_elem *elem = hash_find (&supt->spt, &spte_temp.elem);
    if(elem == NULL) return NULL;
    return hash_entry(elem, struct supp_page_table_entry, elem);
}

bool
vm_spt_has_entry (struct supp_page_table *supt, void *page)
{
    /* Find the SUPT entry. If not found, it is an unmanaged page. */
    struct supp_page_table *spte = vm_spt_lookup(supt, page);
    if(spte == NULL) return false;

    return true;
}

bool
vm_spt_mm_unmap(
        struct supp_page_table *supt, uint32_t *pagedir,
        void *page, struct file *f, off_t offset)
{
    file_seek(f, offset);

    struct supp_page_table_entry *spte = vm_spt_lookup(supt, page);
    if(spte == NULL) {
        PANIC ("munmap - some page is missing; can't happen!");
    }
#if 0
    printf("[unmap] spte = %x (status = %d), upage = %x (%d)\n",
      spte, spte->loc, spte->uaddr, spte->uaddr);
#endif

    // TODO pin the frame if exists

    // see also, vm_load_page()
    switch (spte->loc)
    {
        case FRAME:
        {
            // Dirty frame handling (write into file)
            // Check if the upage or mapped frame is dirty. If so, write to file.
            bool is_dirty = false;
            is_dirty = is_dirty || pagedir_is_dirty(pagedir, spte->uaddr);

            if(is_dirty) {
                file_write_at (f, spte->uaddr, PGSIZE, offset);
            }

            pagedir_clear_page (pagedir, spte->uaddr);
            break;
        }

        case SWAP:{
           // vm_swap_free (spte->swap_index);
            break;
        }
        case FILE_SYS:{
            // do nothing.
            break;
        }
        default:{
            // Impossible, such as ALL_ZERO
            PANIC ("unreachable state");
        }
    }

    // the supplemental page table entry is also removed.
    // so that the unmapped memory is unreachable. Later access will fault.
    hash_delete(& supt->spt, &spte->elem);
    return true;
}
