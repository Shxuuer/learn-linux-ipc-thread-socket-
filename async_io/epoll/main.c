#include <sys/epoll.h>
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

int main(void)
{
    int console_fd = STDIN_FILENO, epoll_fd = epoll_create1(0);
    set_non_blocked(console_fd);

    struct epoll_event event;
    event.data.fd = console_fd;
    event.events = EPOLLIN | EPOLLET;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, console_fd, &event);

    struct epoll_event events[10];

    while (1)
    {
        int num = epoll_wait(epoll_fd, events, 10, 10); // 等待事件，超时10毫秒
        for (int i = 0; i < num; ++i)                   // 事实上最大只有一个num，console
        {
            int fd = events[i].data.fd; // 获取触发事件的文件描述符
            if (fd == console_fd)
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
    }
    // 删除指定的文件描述符
    // epoll_ctl(epoll_fd, EPOLL_CTL_DEL, console_fd, NULL);
    close(epoll_fd);
    return 0;
}