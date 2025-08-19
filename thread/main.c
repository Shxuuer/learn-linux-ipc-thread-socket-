#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;

void *producer(void *arg)
{
    int *table = (int *)arg;

    for (int j = 0; j < 10; j++)
    {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 10; i++)
        {
            if (table[i] == 0)
            {
                table[i] = 1; // Simulate producing an item
                printf("Produced item at index %d\n", i);
                break; // Exit after producing one item
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    printf("Producer thread finished\n");
    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    int *table = (int *)arg;

    while (1)
    {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < 10; i++)
        {
            if (table[i] == 1)
            {
                table[i] = 0; // Simulate consuming an item
                printf("Consumed item at index %d\n", i);
                break; // Exit after consuming one item
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int main()
{
    int *table = (int *)malloc(10 * sizeof(int));
    pthread_mutex_init(&mutex, NULL);

    pthread_t producer_thread[10], consumer_thread[10];
    for (int i = 0; i < 10; i++)
    {
        if (pthread_create(&consumer_thread[i], NULL, consumer, (void *)table))
        {
            printf("error creating consumer thread\n");
            return 1; // Error creating consumer thread
        }
    }
    for (int i = 0; i < 10; i++)
    {
        if (pthread_create(&producer_thread[i], NULL, producer, (void *)table))
        {
            printf("error creating producer thread\n");
            return 1; // Error creating producer thread
        }
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(producer_thread[i], NULL);
        pthread_detach(consumer_thread[i]);
    }

    pthread_mutex_destroy(&mutex);
    free(table);

    return 0;
}