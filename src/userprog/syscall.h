#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "user/syscall.h"
#include "userprog/process.h"

#define SYSCALL_TOTAL 15



void syscall_init (void);

/* syscall wrappers */
int _syscall_halt (struct intr_frame *f);
int _syscall_exit (struct intr_frame *f);
int _syscall_exec (struct intr_frame *f);
int _syscall_wait (struct intr_frame *f);
int _syscall_create (struct intr_frame *f);
int _syscall_remove (struct intr_frame *f);
int _syscall_open (struct intr_frame *f);
int _syscall_filesize (struct intr_frame *f);
int _syscall_read (struct intr_frame *f);
int _syscall_write (struct intr_frame *f);
int _syscall_seek (struct intr_frame *f);
int _syscall_tell (struct intr_frame *f);
int _syscall_close (struct intr_frame *f);
int _syscall_mmap (struct intr_frame *f);
int _syscall_munmap (struct intr_frame *f);

//user implemented methods
void syscall_halt(void);
void syscall_exit(int status);
pid_t syscall_exec(const char* cmd_line);
int syscall_wait(pid_t pid);
bool syscall_create(const char* file,unsigned initial_size);
bool syscall_remove(const char* file);
int syscall_open(const char* file);
int syscall_filesize(int fd);
int syscall_read(int fd,void* buffer, unsigned size);
int syscall_write(int fd, const void *buffer,unsigned size);
void syscall_seek(int fd,unsigned position);
unsigned syscall_tell(int fd);
void syscall_close(int fd);
bool syscall_munmap(mmapid_t mid);
mmapid_t syscall_mmap(int fd, void *upage);


#endif /* userprog/syscall.h */
