#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "http.h"

volatile sig_atomic_t is_running = 1;

int create_server_socket(int port);
void sig_handler(int sig);
int set_non_blocking(int fd);

int main()
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // 创建HTTP服务器套接字，监听端口8080
    int server_fd = create_server_socket(8080);
    if (server_fd < 0)
    {
        fprintf(stderr, "Failed to create server socket\n");
        return EXIT_FAILURE;
    }
    set_non_blocking(server_fd);

    // create a epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
    {
        fprintf(stderr, "Failed to create epoll instance\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0)
    {
        perror("epoll_ctl");
        close(server_fd);
        close(epoll_fd);
        return EXIT_FAILURE;
    }

    // 就绪的事件列表
    struct epoll_event events[1000];

    while (is_running)
    {
        int nfds = epoll_wait(epoll_fd, events, 10, 100);

        // 处理每个就绪的事件
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                // 新连接到来
                int connection_fd;
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);

                while ((connection_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) >= 0)
                {
                    set_non_blocking(connection_fd);
                    // printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    // 将新连接添加到epoll中
                    event.events = EPOLLIN | EPOLLET; // 边缘触发
                    event.data.fd = connection_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &event) < 0)
                    {
                        perror("epoll_ctl add");
                        close(connection_fd);
                        continue;
                    }
                    // printf("Added fd %d to epoll\n", connection_fd);
                }
            }
            else
            {
                // 处理HTTP请求
                // printf("Handling request on fd %d\n", events[i].data.fd);
                handle_http_request(events[i].data.fd);
                // 从epoll中删除连接
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) < 0)
                {
                    perror("epoll_ctl del");
                }
                close(events[i].data.fd);
                // printf("Closed connection on fd %d\n", events[i].data.fd);
            }
        }
    }

    // 清理资源
    close(server_fd);
    close(epoll_fd);
    return EXIT_SUCCESS;
}

int set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl get flags");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("fcntl set non-blocking");
        return -1;
    }
}

void sig_handler(int sig)
{
    is_running = 0;
    exit(0);
}

int create_server_socket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}