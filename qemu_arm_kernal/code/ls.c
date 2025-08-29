// ls.c
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    const char *path = ".";
    if (argc > 1)
    {
        path = argv[1];
    }

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir failed");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // 获取文件类型
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat fileStat;
        if (stat(fullPath, &fileStat) == -1)
        {
            perror("stat failed");
            closedir(dir);
            return 1;
        }

        // 打印文件类型和名称
        if (S_ISDIR(fileStat.st_mode))
            printf("d ");
        else if (S_ISREG(fileStat.st_mode))
            printf("- ");
        else
            printf("? ");

        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}