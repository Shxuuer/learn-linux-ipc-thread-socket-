#include "thread_pool.h"

// 初始化任务数组
int init_tkq(task_queue_t *tkq, int capacity)
{
    tkq->task_capacity = capacity;
    tkq->hptr = 0;
    tkq->tptr = 0;
    tkq->connection_fd = malloc(capacity * sizeof(int));
    for (int i = 0; i < capacity; i++)
    {
        tkq->connection_fd[i] = -1;
    }
    pthread_mutex_init(&tkq->mutex, NULL);
    pthread_cond_init(&tkq->not_empty, NULL);
    pthread_cond_init(&tkq->not_full, NULL);
    return 0;
}

/**
 * 从任务数组中获取任务
 * @param tkq 任务数组指针
 * @return <1失败，否则为connection_fd
 */
int pop_task(task_queue_t *tkq)
{
    pthread_mutex_lock(&tkq->mutex);
    while (tkq->hptr == tkq->tptr)
    {
        pthread_cond_wait(&tkq->not_empty, &tkq->mutex);
    }
    int connection_fd = tkq->connection_fd[tkq->hptr];
    tkq->connection_fd[tkq->hptr] = -1; // 清除任务
    tkq->hptr = (tkq->hptr + 1) % tkq->task_capacity;

    pthread_cond_signal(&tkq->not_full);
    pthread_mutex_unlock(&tkq->mutex);
    return connection_fd;
}

void push_task(task_queue_t *tkq, int connection_fd)
{
    pthread_mutex_lock(&tkq->mutex);
    while ((tkq->tptr + 1) % tkq->task_capacity == tkq->hptr)
    {
        pthread_cond_wait(&tkq->not_full, &tkq->mutex);
    }
    tkq->connection_fd[tkq->tptr] = connection_fd;
    tkq->tptr = (tkq->tptr + 1) % tkq->task_capacity;

    pthread_cond_signal(&tkq->not_empty);
    pthread_mutex_unlock(&tkq->mutex);
}

void *thread_function(void *queue)
{
    task_queue_t *tkq = (task_queue_t *)queue;
    while (is_running)
    {
        int connection_fd = pop_task(tkq);
        handle_http_request(connection_fd);
    }
    pthread_exit(NULL);
}

/**
 * 初始化并创建线程池
 * @param pool 线程池指针
 * @param num_threads 线程数量
 * @return 0 成功，-1 失败
 */
int init_thread_pool(thread_pool_t *pool, int num_threads, task_queue_t *tkq)
{
    pool->threads = malloc(num_threads * sizeof(pthread_t));
    if (!pool->threads)
    {
        return -1;
    }
    pool->num_threads = num_threads;

    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_create(&pool->threads[i], NULL, thread_function, tkq) != 0)
        {
            free(pool->threads);
            return -1;
        }
    }
    return 0;
}

void close_thread_pool(thread_pool_t *pool)
{
    for (int i = 0; i < pool->num_threads; i++)
    {
        pthread_join(pool->threads[i], NULL);
    }
    free(pool->threads);
    free(pool);
}

void close_task_queue(task_queue_t *tkq)
{
    pthread_mutex_destroy(&tkq->mutex);
    pthread_cond_destroy(&tkq->not_empty);
    pthread_cond_destroy(&tkq->not_full);
    free(tkq->connection_fd);
    free(tkq);
}