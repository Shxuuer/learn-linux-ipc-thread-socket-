#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 64

int main()
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, SHM_SIZE) == -1)
    {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    char *ptr = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    scanf("%s", ptr);

    msync(ptr, SHM_SIZE, MS_SYNC);
    munmap(ptr, SHM_SIZE);
    close(fd);

    // 暂时不删除共享内存对象
    // shm_unlink(SHM_NAME);
    return 0;
}