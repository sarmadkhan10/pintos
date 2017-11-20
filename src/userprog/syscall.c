#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

/* Checks the validity of the user vaddr. Returns true if uaddr is not
   null, is not a pointer to kernel vitrual addr. space and is not a
   pointer to unmapped memory. */
static bool
is_uaddr_valid (void *uaddr)
{
  bool valid = false;

  if ((uaddr != NULL) && (is_user_vaddr (uaddr)))
    {
      if (pagedir_get_page (thread_current()->pagedir, uaddr) != NULL)
	{
	  valid = true;
	}
    }

  return valid;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void 
syscall_halt(void)
{
	//TODO: to implement
}

void 
syscall_exit(int status)
{
	//TODO: to implement
}


pid_t 
syscall_exec(const char* cmd_line)
{
	//TODO: to implement
	return 0;
}


int 
syscall_wait(pid_t pid)
{
	//TODO: to implement
	return 0;
}


bool 
syscall_create(const char* file,unsigned initial_size)
{
	//TODO: to implement
	return false;
}


bool 
syscall_remove(const char* file)
{
	//TODO: to implement
	return false;
}


int 
syscall_open(const char* file)
{
	//TODO: to implement
	return 0;
}


int 
syscall_filesize(int fd)
{
	//TODO: to implement
	return 0;
}


int 
syscall_read(int fd,void* buffer, unsigned size)
{
	//TODO: to implement
	return 0;
}


int 
syscall_write(int fd, const void *buffer,unsigned size)
{
	//TODO: to implement
	return 0;
}

void 
syscall_seek(int fd,unsigned position)
{
	//TODO: to implement
}


unsigned 
syscall_tell(int fd)
{
	//TODO: to implement
	return 0;
}


void 
syscall_close(int fd)
{
	//TODO: to implement
}



static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}
