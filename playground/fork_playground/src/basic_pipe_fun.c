#include <stdio.h>
#include <string.h>
#include <unistd.h> //use for pipe
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    int fd[2];
    // fd[0] - read
    // fd[1] - write
    if (pipe(fd) == -1){
        printf("Error occured with opening the pipe!\n");
        return 1;
    }
    int pid = fork();

    if (pid == -1) {
        printf("Error forking!\n");
        return 4;
    }

    if (pid ==0) {
        close(fd[0]);
        int x;
        printf("Input a number: ");
        scanf("%d", &x);
        if (write(fd[1], &x, sizeof(int)) == -1) {
            printf("Error occured while writing to pipe!\n");
            return 2;
        }
        close(fd[1]); 
    } else {
        close(fd[1]);
        int y;
        if (read(fd[0], &y, sizeof(int)) == -1) {
            printf("Error occured while reading from pipe!\n");
            return 3;
        }
        close(fd[0]);
    
        printf("Added 1 to child process %d\n", ++y);
    }

    return 0;
}
