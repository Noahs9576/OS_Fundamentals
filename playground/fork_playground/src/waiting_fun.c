#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    int c_pid;
    int n;

    if (pid == 0) {
        n = 1;
    } else {
        wait(NULL);
        n = 6;
    }

    for (int i = n; i < n + 5; i++) {
        printf("%d ", i);
    }

    if (pid != 0) printf("\n");

    return 0;
}
