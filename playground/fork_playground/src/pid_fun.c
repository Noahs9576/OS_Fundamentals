#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    
    if (pid == 0) sleep(1);
    printf("Current ID: %d, parent ID: %d\n", getpid(), getppid());

    if (pid != 0) {
        int res = wait(NULL);
        res == -1 ? printf("Process %d does not have any childeren to wait for!\n", getpid()) : printf("Child process %d finished!\n", res);
    }

    return 0;
}
