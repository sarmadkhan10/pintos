#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

//lock for File-System allowing one thread access per time
struct lock filesystem_lock;

//list of files opened by users-progs
struct list useropened_files;

static void syscall_handler (struct intr_frame *);
int (*syscall_table[SYSCALL_TOTAL]) (struct intr_frame *);

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

static struct file_descriptor *
get_open_file (int fd)
{
  struct list_elem *elem;
  struct file_descriptor *file_descriptor;
  elem = list_tail (&useropened_files);
  while ((elem = list_prev (elem)) != list_head (&useropened_files))
    {
      file_descriptor = list_entry (elem, struct file_descriptor, element);
      if (file_descriptor->descriptor_id == fd)
	{
	  return file_descriptor;
	}
    }
  return NULL;
}


void
syscall_init (void) 
{
  syscall_table[SYS_HALT] = _syscall_halt;
  syscall_table[SYS_EXIT] = _syscall_exit;
  syscall_table[SYS_EXEC] = _syscall_exec;
  syscall_table[SYS_WAIT] = _syscall_wait;
  syscall_table[SYS_CREATE] = _syscall_create;
  syscall_table[SYS_REMOVE] = _syscall_remove;
  syscall_table[SYS_OPEN] = _syscall_open;
  syscall_table[SYS_FILESIZE] = _syscall_filesize;
  syscall_table[SYS_READ] = _syscall_read;
  syscall_table[SYS_WRITE] = _syscall_write;
  syscall_table[SYS_SEEK] = _syscall_seek;
  syscall_table[SYS_TELL] = _syscall_tell;
  syscall_table[SYS_CLOSE] = _syscall_close;
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

int
_syscall_halt (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_exit (struct intr_frame *f)
{
  if (is_uaddr_valid (((int *)f->esp) + 1) == false)
    {
      // kill process and exit
    }

  int status = *((int *)(f->esp) + 1);

  syscall_exit (status);

  return 0;
}

int
_syscall_exec (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_wait ( struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_create (struct intr_frame *f UNUSED)
{
  //TODO: to implement
    return false;
}

int
_syscall_remove (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return false;
}

int
_syscall_open (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_filesize (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_read (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_write (struct intr_frame *f)
{
  int fd;
  char* buffer;
  unsigned size;
  int status;
  int num_bytes;

  fd = *((int *)f->esp + 1);
  buffer = *(char **)f->esp + 8;
  size = *((unsigned *)f->esp + 3);

  num_bytes = syscall_write (fd, buffer, size);

  f->eax = num_bytes;

  return 0;
}

int
_syscall_seek (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_tell (struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}

int
_syscall_close(struct intr_frame *f UNUSED)
{
  //TODO: to implement
  return 0;
}


void
syscall_halt(void)
{
  shutdown_power_off ();
}


void 
syscall_exit (int status)
{
  thread_exit (status);
}


pid_t 
syscall_exec(const char* cmd_line UNUSED)
{
  //TODO: to implement
  return 0;
}


int 
syscall_wait(pid_t pid UNUSED)
{
  //TODO: to implement
  return 0;
}


bool 
syscall_create(const char* file UNUSED,unsigned initial_size UNUSED)
{
  //TODO: to implement
  return false;
}


bool 
syscall_remove(const char* file UNUSED)
{
  //TODO: to implement
  return false;
}


int 
syscall_open(const char* file UNUSED)
{
  //TODO: to implement
  return 0;
}


int 
syscall_filesize(int fd UNUSED)
{
  //TODO: to implement
  return 0;
}


int 
syscall_read(int fd UNUSED,void* buffer UNUSED, unsigned size UNUSED)
{
  //TODO: to implement
  return 0;
}


int 
syscall_write(int fd, const void *buffer,unsigned size)
{
  int status = 0;
    struct file_descriptor *file_descriptor;
    unsigned temp_buffer_size = size;
    void *temp_buffer = buffer;

    //valid memory check
    while (temp_buffer != NULL)
      {
        if (!is_uaddr_valid (temp_buffer))
  	{
  	  syscall_exit (-1);
  	}
        else
  	{
  	  //termination condition
  	  if (temp_buffer_size == 0)
  	    {
  	      temp_buffer = NULL;
  	    }
  	  else if (temp_buffer_size > PGSIZE)
  	    {
  	      temp_buffer = temp_buffer + PGSIZE;
  	      temp_buffer = temp_buffer - PGSIZE;
  	    }

  	  else
  	    {
  	      temp_buffer = buffer + size - 1;
  	      temp_buffer_size = 0;
  	    }
  	}
      }

    lock_acquire (&filesystem_lock);
    if (fd == STDIN_FILENO)
      {
        status = -1;
      }
    else if (fd == STDOUT_FILENO)
      {
        putbuf (buffer, size);
        status = size;
      }
    else
      {
        file_descriptor = get_open_file (fd);
        if (file_descriptor != NULL)
  	{
  	  status = file_write (file_descriptor->file, buffer, size);
  	}
      }
    lock_release (&filesystem_lock);

    return status;

}

void 
syscall_seek(int fd UNUSED,unsigned position UNUSED)
{
  //TODO: to implement
}


unsigned 
syscall_tell(int fd UNUSED)
{
  //TODO: to implement
  return 0;
}


void 
syscall_close(int fd UNUSED)
{
  //TODO: to implement
}


static void
syscall_handler (struct intr_frame *f)
{
  //printf ("system call!\n");
  //thread_exit ();

  if (is_uaddr_valid (f) == false)
    {
      // exit and kill process
    }

  /* get the syscall no. from the stack. */
  int syscall_no = *((int *)(f->esp));

  /* invoke syscall */
  syscall_table[syscall_no] (f);
}
