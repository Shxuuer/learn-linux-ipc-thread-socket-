#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

void cleanup(int sgn)
{
    mq_unlink("/my_mq");
    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    mqd_t mq = mq_open("/my_mq", O_CREAT | O_RDONLY, 0666, NULL);
    if (mq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    char buffer[10240];
    while (1)
    {
        if (mq_receive(mq, buffer, 10240, NULL) == -1)
        {
            perror("mq_receive");
            exit(EXIT_FAILURE);
            mq_close(mq);
        }
        printf("Received: %s\n", buffer);
        sleep(3);
    }
}