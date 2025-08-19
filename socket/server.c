#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>

int socket_fd;

void clear_up(int sig)
{
    if (socket_fd >= 0)
    {
        close(socket_fd);
    }
    exit(0);
}

int main(void)
{

    signal(SIGINT, clear_up);

    // 创建socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 绑定地址到socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接，队列长度为5
    if (listen(socket_fd, 5) < 0)
    {
        perror("listen");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // 接受连接
    while (1)
    {
        int connection_fd;
        if ((connection_fd = accept(socket_fd, NULL, NULL)) < 0)
        {
            perror("accept");
            continue;
        }
        printf("Client connected from %s\n", inet_ntoa(server_addr.sin_addr));
        char buffer[1024] = {0};
        while (strcmp(buffer, "exit") != 0)
        {
            ssize_t bytes_received = recv(connection_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0)
            {
                perror("recv");
                close(connection_fd);
                continue;
            }
            buffer[bytes_received] = '\0'; // 确保字符串结束
            printf("Received: %s\n", buffer);
        }

        close(connection_fd);
    }

    return 0;
}