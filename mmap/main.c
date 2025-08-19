#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define MEMORY_SIZE 64

int main()
{
    int fd = open("input.txt", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }
    ftruncate(fd, MEMORY_SIZE);

    // PROT_READ可读，PROT_WRITE可写
    // MAP_SHARED表示映射的内存区域可以被多个进程共享，会同步到文件
    // MAP_PRIVATE表示映射的内存区域是私有的，修改不会影响
    char *ptr = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Mapped content: %s\n", ptr);
    // 修改映射的内容
    char *inner = "Hello, mmap!";
    strcpy(ptr, inner);
    printf("Modified content: %s\n", ptr);

    msync(ptr, MEMORY_SIZE, MS_SYNC);

    if (munmap(ptr, MEMORY_SIZE) == -1)
    {
        perror("munmap");
        close(fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}