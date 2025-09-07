#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

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
    int console_fd = STDIN_FILENO;
    set_non_blocked(console_fd);

    fd_set read_fds;
    struct timeval timeout;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(console_fd, &read_fds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; // 10毫秒

        int ret = select(console_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0)
        {
            perror("select error");
            continue;
        }
        else if (ret == 0)
        {
            // 超时，无事件发生
            continue;
        }

        if (FD_ISSET(console_fd, &read_fds)) // 检查是否有数据可读
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