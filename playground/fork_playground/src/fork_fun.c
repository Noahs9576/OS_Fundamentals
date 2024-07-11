#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int pid = fork();

    printf("Hello World from: %s\n", pid == 0 ? "child" : "parent");
    return 0;
}
