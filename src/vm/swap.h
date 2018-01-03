#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <bitmap.h>

#define SWAP_FULL   BITMAP_ERROR

void    swap_init (void);
size_t  swap_write_to_unused_slot (void *);
void    swap_read_from_slot (size_t , void *);

#endif /* VM_SWAP_H */
