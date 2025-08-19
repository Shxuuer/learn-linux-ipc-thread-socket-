#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 64

int main(void)
{
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    ftruncate(fd, SHM_SIZE);

    char *ptr = mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Read from shared memory: %s\n", ptr);
    munmap(ptr, SHM_SIZE);
    close(fd);
    shm_unlink(SHM_NAME); // 删除共享内存对象
    return 0;
}
