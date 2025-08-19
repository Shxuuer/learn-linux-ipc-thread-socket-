#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    mqd_t mq = mq_open("/my_mq", O_WRONLY, 0666, NULL);
    if (mq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    int priority;
    printf("Enter priority and message (e.g., 1 Hello):\n");
    while (1)
    {
        scanf("%d %s", &priority, buffer);
        if (mq_send(mq, buffer, 1024, priority) == -1)
        {
            perror("mq_send");
            exit(EXIT_FAILURE);
            return 0;
        }
    }
    mq_close(mq);
}