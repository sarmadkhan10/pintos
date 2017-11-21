#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "userprog/syscall.h"

struct process_info
  {
    tid_t tid;          /* tid of the process */
    tid_t parent_tid;
    int status_code;    /* set by exit () syscall */
    struct list_elem elem;
  };

void process_init (void);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (int status);
void process_activate (void);

#endif /* userprog/process.h */
