#include "threads/thread.h"
#include "vm/page.h"
#include <hash.h>
#include "threads/malloc.h"

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

static unsigned
spt_hash_func(const struct hash_elem *elem, void *aux UNUSED)
{
  struct supp_page_table_entry *entry = hash_entry (elem, struct supp_page_table_entry, elem);
  return hash_bytes (&entry->uaddr, sizeof (entry->uaddr));
}

static bool
spt_less_hash_func(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  struct supp_page_table_entry *a_entry = hash_entry (a, struct supp_page_table_entry, elem);
  struct supp_page_table_entry *b_entry = hash_entry (b, struct supp_page_table_entry, elem);
  return a_entry->uaddr < b_entry->uaddr;
}
