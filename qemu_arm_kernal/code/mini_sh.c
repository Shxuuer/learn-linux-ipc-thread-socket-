#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int spliceCmdAndArgs(char *input, char **cmd, char **args)
{
    char *token = strtok(input, " ");
    if (token == NULL)
        return -1;

    *cmd = token;
    int i = 0;
    while (token != NULL && i < 100)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    return 0;
}

int main()
{
    char buffer[1024];
    while (1)
    {
        printf("$");
        scanf("%[^\n]%*c", buffer);
        buffer[strcspn(buffer, "\n")] = 0; // 去掉换行符
        if (fork() == 0)
        {
            char *cmd;
            char *args[100];
            if (spliceCmdAndArgs(buffer, &cmd, args) != 0)
            {
                perror("Failed to parse command");
                return 0;
            }
            execv(cmd, args);
            perror("exec failed");
            return 0;
        }
        wait(NULL);
    }
}