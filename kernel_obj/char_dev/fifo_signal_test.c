#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int fd = -1;

void handle_io_signal(int sig)
{
    char buf[1024];

    read(fd, buf, sizeof(buf));
    printf("Received data: %s\n", buf);
}

int main(void)
{
    fd = open("/dev/fifo", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    fcntl(fd, __F_SETOWN, getpid());
    int oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | O_ASYNC);
    signal(SIGIO, handle_io_signal);
    while (1)
        ;
    return 0;
}