#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argv, char *argc[])
{
    char buffer[20];
    int fd;
    fd = open("fifo_test", O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    while (1)
    {
        scanf("%s", buffer);
        write(fd, buffer, sizeof(buffer));
        printf("Sent %s\n", buffer);
    }

    return 0;
}