#include "vm/swap.h"
#include "threads/thread.h"
#include "devices/block.h"
#include <bitmap.h>
#include <stdio.h>
#include "threads/vaddr.h"

static struct block *swap_block;

/* no. of sectors in swap block */
static block_sector_t swap_size;

/* bitmap for availability of swap slots. 1: free. 0: taken */
static struct bitmap *swap_bitmap;

#define SECTORS_PER_PAGE  PGSIZE / BLOCK_SECTOR_SIZE


void
swap_init (void)
{
  swap_block = block_get_role (BLOCK_SWAP);

  if (swap_block == NULL)
    {
      /* Panic the kernel since swap_block should not be null */
      PANIC ("Unable to retrieve swap block device.");
    }

  /* no. of sectors in swap block */
  swap_size = block_size (swap_block);

  swap_bitmap = bitmap_create (swap_size);

  if (swap_bitmap == NULL)
    {
      printf ("swap bitmap alloc failed. Out of mem.\n");
      NOT_REACHED ();
    }

    /* in the start, all slots are available */
    bitmap_set_all (swap_bitmap, true);
}

/* writes to a (unused) slot in swap. and returns the index (in swap_bitmap). if swap is
   full, return SWAP_FULL. extern sync required */
size_t
swap_write_to_unused_slot (void *page)
{
  ASSERT (page >= PHYS_BASE);
  size_t start = bitmap_scan (swap_bitmap, 0, SECTORS_PER_PAGE, true);

  if (start != BITMAP_ERROR)
    {
      /* swap is not full, the page can be written */

      int i;
      for (i = 0; i < SECTORS_PER_PAGE; i++)
        {
          /* set the sector in swap as taken */
          bitmap_flip (swap_bitmap, start + i);

          block_write (swap_block, start + i, page + (BLOCK_SECTOR_SIZE * i));
        }
    }
  else
    {
      /* swap is full */
      printf ("swap is full.\n");
    }

  return start;
}

/* reads page from start_index sector to buff and frees the slot in swap.
   buff must be 4kB or 4 sectors' sized */
void
swap_read_from_slot (size_t start_index, void *page)
{
  ASSERT (start_index < swap_size);
  ASSERT (page >= PHYS_BASE);

  /* verify the region start_index to start_index + SECTORS_PER_PAGE in swap block is marked as taken */
  if (bitmap_any ((const struct bitmap *) swap_bitmap, start_index, SECTORS_PER_PAGE))
    {
      /* this shouldn't happen */
      NOT_REACHED ();
    }

  int i;
  for (i = 0; i < SECTORS_PER_PAGE; i++)
    {
      block_read (swap_block, start_index + i, page);
    }

  /* once we have read the 4 sectors, we can mark these as free */
  bitmap_set_multiple (swap_bitmap, start_index, SECTORS_PER_PAGE, true);
}
