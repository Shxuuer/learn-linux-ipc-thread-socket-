#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

int fd;

void cleanup(int sig)
{
    close(fd);
    printf("Cleaning up...\n");
    unlink("fifo_test");
}

int main()
{
    signal(SIGINT, cleanup);

    char buffer[20];
    // Create FIFO
    if (mkfifo("fifo_test", 0666) == -1)
    {
        perror("mkfifo");
        return -1;
    }

    // Open FIFO for reading
    fd = open("fifo_test", O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    while (1)
    {
        read(fd, buffer, sizeof(buffer));
        printf("Received %s\n", buffer);
    }
    return 0;
}