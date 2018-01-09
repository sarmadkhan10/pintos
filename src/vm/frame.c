#include <hash.h>
#include "lib/kernel/hash.h"
#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include "vm/page.h"

/* A global lock, to ensure critical sections on frame operations. */
static struct lock frame_lock;

/* A mapping from physical address to frame table entry. */
static struct hash frame_map;

static void     *vm_frame_evict (enum palloc_flags flags);
static unsigned frame_hash_func(const struct hash_elem *elem, void *aux);
static bool     frame_less_func(const struct hash_elem *, const struct hash_elem *, void *aux);

static struct frame_table_entry *vm_frame_find (void *kvaddr);

/**
 * Frame Table Entry
 */
struct frame_table_entry
  {
    void *kvaddr;               /* kernel virtual address */
    void *uvaddr;               /* user virtual address (only needed for eviction) */
    struct hash_elem elem;      /* see frame_map */
    struct thread *t;           /* The associated thread. */
  };


void
vm_frame_init ()
{
  lock_init (&frame_lock);
  hash_init (&frame_map, frame_hash_func, frame_less_func, NULL);
}

/**
 * Allocate a new frame, and return the address of the associated page
 * on user's virtual memory.
 */
void*
vm_frame_allocate (enum palloc_flags flags, void *paddr)
{
  void *vpage = palloc_get_page (PAL_USER | flags);
  if (vpage == NULL)
    {
      /* frame allocation failed. evict a frame to make space */
      vpage = vm_frame_evict (flags);

      if (vpage == NULL)
        PANIC ("Failed to get a frame after eviction");
    }

  struct frame_table_entry *frame = malloc(sizeof(struct frame_table_entry));
  if(frame == NULL)
    {
      // frame allocation failed
      PANIC ("out of mem: vm_frame_allocate");
    }

  frame->t = thread_current ();
  frame->kvaddr = vpage;
  frame->uvaddr = paddr;

  // insert into hash table
  lock_acquire (&frame_lock);
  hash_insert (&frame_map, &frame->elem);
  lock_release (&frame_lock);

  return vpage;
}

/**
 * Deallocate a frame or page.
 */
void
vm_frame_free (void *vpage)
{
  /* should be page aligned */
  if (((uintptr_t) vpage & PGMASK) != 0)
    {
      PANIC ("vm_frame_free is not aligned - aborting");
    }

  // hash lookup : a temporary entry
  struct frame_table_entry *f = (struct frame_table_entry*) malloc(sizeof(struct frame_table_entry));
  f->kvaddr = vpage;

  struct hash_elem *h = hash_find (&frame_map, &f->elem);
  free(f);
  if (h == NULL) {
    PANIC ("The page to be freed is not stored in the table");
  }

  f = hash_entry(h, struct frame_table_entry, elem);

  lock_acquire (&frame_lock);
  hash_delete (&frame_map, &f->elem);
  lock_release (&frame_lock);

  // Free resources
  palloc_free_page (f->kvaddr);
  free(f);
}

static void *
vm_frame_evict (enum palloc_flags flags)
{
  struct hash_iterator i;
  void *frame_to_evict = NULL;
  bool clean_page_found = false;
  bool written_to_swap = false;
  size_t swap_index;

  lock_acquire (&frame_lock);

  hash_first (&i, &frame_map);
  while (hash_next (&i))
    {
      struct frame_table_entry *frame_entry  = hash_entry (hash_cur (&i), struct frame_table_entry, elem);

      /* check if the frame entry belongs to the current thread */
      void *temp = pagedir_get_page (thread_current ()->pagedir, frame_entry->uvaddr);
      if (temp == NULL)
          continue;

      /* if a clean page is found, evict it directly (don't need to write to swap) */
      if (pagedir_is_dirty (thread_current ()->pagedir, frame_entry->uvaddr) == false)
        {
          frame_to_evict = frame_entry->kvaddr;
          clean_page_found = true;
          break;
        }
    }

  /* if a clean is not found, just evict randomly to swap */
  if (!clean_page_found)
    {
      printf ("here");
      hash_first (&i, &frame_map);
      if (hash_next (&i))
        {
          struct frame_table_entry *frame_entry  = hash_entry (hash_cur (&i), struct frame_table_entry, elem);

          frame_to_evict = frame_entry->kvaddr;

          swap_index = swap_write_to_unused_slot (frame_to_evict);

          if (swap_index == SWAP_FULL)
            PANIC ("SWAP is full");
          written_to_swap = true;

          pagedir_set_dirty (thread_current ()->pagedir, frame_to_evict, false);
        }
      else
        NOT_REACHED ();
    }

  struct frame_table_entry *frame_evict_entry = vm_frame_find (frame_to_evict);

  struct supp_page_table_entry *spt_e = spt_find_page (thread_current ()->spt, frame_evict_entry->uvaddr);

  if (written_to_swap)
    {
      spt_e->loc = SWAP;
      spt_e->swap_index = swap_index;
    }
  else
    {
      spt_e->loc = FILE_SYS;
    }

  /* we found the entry, we can release the frame lock */
  lock_release (&frame_lock);

  void *frame_uaddr = frame_evict_entry->uvaddr;

  vm_frame_free (frame_evict_entry->kvaddr);

  pagedir_clear_page (thread_current ()->pagedir, frame_uaddr);

  return palloc_get_page (PAL_USER | flags);
}

/* finds the frame table entry in the frame table given the kvaddr */
static struct frame_table_entry *
vm_frame_find (void *kvaddr)
{
  ASSERT (kvaddr != NULL);

  struct frame_table_entry frame_entry;
  frame_entry.kvaddr = kvaddr;
  struct hash_elem *e;

  e = hash_find (&frame_map, &frame_entry.elem);
  struct frame_table_entry *frame_found = hash_entry (e, struct frame_table_entry, elem);

  if (frame_found == NULL)
    NOT_REACHED ();

  return frame_found;
}

// Hash Functions required for [frame_map]. Uses 'kaddr' as key.
static unsigned frame_hash_func(const struct hash_elem *elem, void *aux UNUSED)
{
  struct frame_table_entry *entry = hash_entry(elem, struct frame_table_entry, elem);
  return hash_int ((int) entry->kvaddr);
}
static bool frame_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct frame_table_entry *a_entry = hash_entry(a, struct frame_table_entry, elem);
  struct frame_table_entry *b_entry = hash_entry(b, struct frame_table_entry, elem);
  return a_entry->kvaddr < b_entry->kvaddr;
}
