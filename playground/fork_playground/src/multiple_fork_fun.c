#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int pid1 = fork();
    int pid2 = fork();

    if (pid1 == 0) {
        if (pid2 == 0) {
            printf("We are process y\n");
        } else {
            printf("We are process x\n");
        }
    } else {
        if (pid2 == 0) {
            printf("We are process z\n");
        } else {
            printf("We are the parent\n");
        }
    }

    // Cannot use just wait! Wait only waits for one process!!!!!
    // wait(NULL);

    while(wait(NULL) != -1 || errno != ECHILD) {
        printf("Waited for a child to finish!\n");
    }

    return 0;
}
