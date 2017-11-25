#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "userprog/syscall.h"

struct process_info
  {
    tid_t tid;          /* tid of the process */
    tid_t parent_tid;   /* tid of parent process */
    int status_code;    /* set by exit () syscall */
    struct list_elem elem;
  };

struct process_wait_info
  {
    struct semaphore sema;
    tid_t waiting_for_tid;
    struct list_elem elem;
  };

/* for filesystem */
struct process_file
  {
    struct file *file;
    int fd;
    struct list_elem elem;
  };

void process_init (void);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (int status);
void process_activate (void);

//added for filesystem
struct file* process_get_file (int fd);
int process_add_file(struct file *f);
#endif /* userprog/process.h */
