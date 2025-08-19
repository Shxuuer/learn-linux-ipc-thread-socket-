#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    char buffer[20];
    int fd[2];

    // 单个进程内的管道通信
    pipe(fd);

    write(fd[1], "Hello, Pipe!", 13);
    read(fd[0], buffer, 13);
    printf("Read from pipe: %s\n", buffer);

    close(fd[0]);
    close(fd[1]);

    // 子进程向父进程通信
    pipe(fd);

    pid_t pid = fork();
    if (!pid) // child
    {
        close(fd[0]);
        write(fd[1], "Message from child", 19);
        close(fd[1]);
        return 0;
    }
    close(fd[1]);
    // wait(NULL);
    read(fd[0], buffer, 19);
    printf("Parent read: %s\n", buffer);

    close(fd[0]);

    // 父进程向子进程通信
    pipe(fd);
    pid = fork();
    if (!pid) // child
    {
        close(fd[1]);
        read(fd[0], buffer, 19);
        printf("Child read: %s\n", buffer);
        close(fd[0]);
        return 0;
    }
    close(fd[0]);
    write(fd[1], "Message from parent", 19);

    close(fd[1]);

    return 0;
}