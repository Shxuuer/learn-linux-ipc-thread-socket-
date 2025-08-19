#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void exit_callback(void)
{
    printf("Exit callback executed.\n");
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        printf("Usage: %s\n", argv[0]);
        return 1;
    }
    // fork
    pid_t pid = fork();
    if (!pid) // 子
    {
        // 注册退出回调函数
        atexit(exit_callback);
        printf("Child process: PID = %d\n", getpid());
        sleep(3);
        exit(0); // 子进程退出
    }
    printf("Parent process: print before child exit\n");
    int status;
    pid = wait(&status); // 不wait会产生僵尸进程
    if (WIFEXITED(status))
    {
        printf("Parent process: Child %d exited with status %d\n", pid, WEXITSTATUS(status));
    }
    else
    {
        printf("Parent process: Child %d did not exit normally with status %d\n", pid, WTERMSIG(status));
    }
    printf("---------\n");

    // vfork,子进程占用父进程进程空间，只有当子进程调用exec或exit时才会释放
    pid = vfork();
    if (!pid) // 子
    {
        printf("Child process: PID = %d\n", getpid());
        sleep(3);
        exit(0); // 子进程退出
    }
    printf("Parent process: print after child exit\n"); // 不需要wait
    printf("---------\n");

    // exec
    pid = fork();
    if (!pid) // 子
    {
        execl("./process", "process", "123", NULL);
    }
    waitpid(pid, &status, 0); // 0表示阻塞等待，WNOHANG表示非阻塞
    printf("---------\n");

    // 清除所有僵尸进程
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
