#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

#include "thread_pool.h"
#include "http.h"

volatile sig_atomic_t is_running = 1;

int create_server_socket(int port);
void sig_handler(int sig);

int main()
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // 初始化任务队列，容量为30
    task_queue_t tkq;
    init_tkq(&tkq, 10000);
    // 初始化线程池，线程数量为10
    thread_pool_t pool;
    if (init_thread_pool(&pool, 5000, &tkq) != 0)
    {
        fprintf(stderr, "Failed to initialize thread pool\n");
        return 1;
    }
    // 创建HTTP服务器套接字，监听端口8080
    int server_fd = create_server_socket(8080);
    printf("HTTP server is running on port 8080\n");
    while (is_running)
    {
        int connection_fd;
        if ((connection_fd = accept(server_fd, NULL, NULL)) < 0)
        {
            perror("accept");
            continue;
        }
        push_task(&tkq, connection_fd);
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