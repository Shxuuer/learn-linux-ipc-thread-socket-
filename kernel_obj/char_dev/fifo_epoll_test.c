#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    char buffer[1024];

    int fifo_fd = open("/dev/fifo", O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0)
    {
        perror("open fifo");
        return 1;
    }
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
    {
        perror("epoll_create1");
        return 1;
    }

    struct epoll_event fifo_read_event;
    fifo_read_event.data.fd = fifo_fd;
    fifo_read_event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fifo_fd, &fifo_read_event);

    struct epoll_event epoll_active_event[1];

    while (1)
    {
        int actnum = epoll_wait(epoll_fd, epoll_active_event, 1, -1);
        for (int i = 0; i < actnum; ++i)
        {
            int fd = epoll_active_event[i].data.fd;
            if (fd == fifo_fd)
            {
                ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
                if (bytes_read < 0)
                {
                    perror("read error");
                    continue;
                }
                buffer[bytes_read] = '\0';
                printf("Read from fifo: %s", buffer);
            }
        }
    }

    return 0;
}