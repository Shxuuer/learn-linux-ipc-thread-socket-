#include <linux/netlink.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <linux/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = -1;

    struct pollfd pfd[1];
    pfd[0].events = POLLIN;
    pfd[0].fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);

    bind(pfd[0].fd, (struct sockaddr *)&addr, sizeof(addr));
    char buffer[1024];
    while (-1 != poll(pfd, 1, -1))
    {
        int len = recv(pfd[0].fd, buffer, sizeof(buffer), 0);
        buffer[len] = '\0';
        printf("len=%d\n", len);
        printf("%s\n", buffer);
    }

    return 0;
}