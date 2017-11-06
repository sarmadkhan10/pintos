/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

/* buffer size */
#define BUFF_SIZE 3

/* data to produce. */
static char data[] = {'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};

/* condition variables. */
struct condition not_full;
struct condition not_empty;

/* lock */
struct lock lock_prod_cons;

/* buffer */
static char buffer[BUFF_SIZE];

/* current count of the buffer */
static int cnt = 0;
/* head keeps track of the 'index' of the producer */
static int head = 0;
/* tail keeps track of the 'index' of the consumer */
static int tail = 0;

void producer_consumer(unsigned int num_producer, unsigned int num_consumer);
void producer (void *items);
void consumer (void *);


void test_producer_consumer(void)
{
    /*producer_consumer(0, 0);
    producer_consumer(1, 0);
    producer_consumer(0, 1);
    producer_consumer(1, 1);
    producer_consumer(3, 1);
    producer_consumer(1, 3);
    producer_consumer(4, 4);
    producer_consumer(7, 2);
    producer_consumer(2, 7);
    producer_consumer(6, 6);*/
    producer_consumer(3, 3);
    pass();
}


void producer_consumer(UNUSED unsigned int num_producer, UNUSED unsigned int num_consumer)
{
  unsigned int i;

  /* initialize condition variables. */
  cond_init (&not_full);
  cond_init (&not_empty);

  /* initialize lock. */
  lock_init (&lock_prod_cons);

  /* create producer threads. */
  for (i = 0; i < num_producer; i++)
  {
    char name[16];
    snprintf (name, sizeof name, "t_prod %d", i);
    thread_create (name, PRI_DEFAULT, producer, data);
  }

  /* create consumer threads. */
  for (i = 0; i < num_consumer; i++)
  {
    char name[16];
    snprintf (name, sizeof name, "t_cons %d", i);
    thread_create (name, PRI_DEFAULT, consumer, NULL);
  }
}

/* Entry point for the producer thread. Produces character(s)
   from the input passed to it. */
void producer (void* items_to_produce)
{
  char *items_prod = (char *) items_to_produce;
  unsigned int i;

  /* produce data string one char at a time. */
  for (i = 0; i < (sizeof (data) / sizeof (char)); i++)
  {
    lock_acquire (&lock_prod_cons);

    /* if the buffer is full, wait on the condition variable.
       Since Pintos condition variables are Mesa-style, check
       the condition again.*/
    while (cnt == BUFF_SIZE)
      cond_wait (&not_full, &lock_prod_cons);

    /* increment the counter */
    cnt++;

    /* add the item to the buffer */
    buffer[head] = items_prod[i];
    head++;

    /* if end of buffer reached, set head to the first index i.e. 0 */
    if (head == BUFF_SIZE)
      head = 0;

    /* signal the consumer that the buffer is not empty */
    cond_signal (&not_empty, &lock_prod_cons);

    lock_release (&lock_prod_cons);
  }
}

/* Entry point for the consumer thread. Consumes data from the buffer.
   If there is no data to be consumed, the thread suspends. */
void consumer (void *aux UNUSED)
{
  char data_to_cons;

  while (true)
  {
    lock_acquire (&lock_prod_cons);

    /* if the buffer is empty, wait on the condition variable. */
    while (cnt == 0)
      cond_wait (&not_empty, &lock_prod_cons);

    cnt--;

    /* consume data from the from the buffer */
    data_to_cons = buffer[tail];
    tail++;

    /* if end of buffer reached, set tail to the first index i.e. 0 */
    if (tail == BUFF_SIZE)
      tail = 0;

    /* the consumed char */
    printf ("%c", data_to_cons);

    /* singal the producer that the buffer is not full */
    cond_signal (&not_full, &lock_prod_cons);

    lock_release (&lock_prod_cons);
  }
}
