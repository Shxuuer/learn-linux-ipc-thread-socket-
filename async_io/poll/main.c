#include <sys/poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

// 将文件标识符设置为非阻塞模式
void set_non_blocked(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl get flags");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl set non-blocking");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int console_fd = STDIN_FILENO;
    set_non_blocked(console_fd);

    struct pollfd fds[1];
    fds[0].fd = console_fd;
    fds[0].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 1, 10); // 等待事件，超时10毫秒
        if (ret < 0)
        {
            perror("poll error");
            continue;
        }
        else if (ret == 0)
        {
            // 超时，无事件发生
            continue;
        }

        if (fds[0].revents & POLLIN) // 检查是否有数据可读
        {
            char buffer[1024];
            ssize_t bytes_read = read(console_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0)
            {
                perror("read error");
                continue;
            }
            write(STDOUT_FILENO, buffer, bytes_read);
        }
    }

    return 0;
}