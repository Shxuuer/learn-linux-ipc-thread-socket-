#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

void input_handle(int signum)
{
    char buf[1024];
    int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n > 0)
    {
        buf[n] = '\0';
        printf("Received input (SIGIO): %s", buf);
    }
}

void ctrl_c_handle(int signum)
{
    printf("Received Ctrl+C (SIGINT): %d\n", signum);
}

int main()
{
    signal(SIGINT, ctrl_c_handle);

    fcntl(STDIN_FILENO, __F_SETOWN, getpid());
    int oflags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oflags | O_ASYNC);
    signal(SIGIO, input_handle);

    while (1)
        ;
}