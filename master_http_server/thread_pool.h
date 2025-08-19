#ifndef __THREAD_POOL_HH__
#define __THREAD_POOL_HH__

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "http.h"

extern volatile int is_running;

typedef struct
{
    pthread_t *threads;
    int num_threads;
} thread_pool_t;

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int *connection_fd;
    int task_capacity;
    int hptr;
    int tptr;
} task_queue_t;

int init_tkq(task_queue_t *tkq, int capacity);
int init_thread_pool(thread_pool_t *pool, int num_threads, task_queue_t *tkq);
void push_task(task_queue_t *tkq, int connection_fd);
int pop_task(task_queue_t *tkq);

#endif // __THREAD_POOL_HH__