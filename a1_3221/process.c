/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

// Header Files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <float.h> 

// Constants
#define BUFFER_SIZE 25
#define READ 0
#define WRITE 1

// Function that reads in a file and calculates the sum, difference, min, and max then writes it to the pipe
int calculations(const char *filename, int write_fd) {
    // Opens the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No file found named: %s\n", filename);
        return 1;
    }

    double min = DBL_MAX;
    double max = -DBL_MAX;
    double num;

    int cnt = 0;
    // Reads in the file and calculates the sum, difference, min, and max
    while (fscanf(file, "%lf", &num) != EOF) {
        double temp = num;

        if (temp < min) {
            min = temp;
        }

        if (temp > max) {
            max = temp;
        }
        cnt++;
    }

    fclose(file);
    
    // Calculates the sum and difference
    double sum = min + max;
    double dif = min - max;

    // Handles the case where there are no numbers in the file
    if (cnt == 0){
        sum = DBL_MAX;
        dif = -DBL_MAX;
        min = DBL_MAX;
        max = -DBL_MAX;
    }

    // Writes the sum, difference, min, max, and filename to the pipe
    write(write_fd, &min, sizeof(double));
    write(write_fd, &max, sizeof(double));
    write(write_fd, &sum, sizeof(double));
    write(write_fd, &dif, sizeof(double));
    write(write_fd, filename, sizeof(char) * BUFFER_SIZE);

    return 0;
}

int main(int argc, char *argv[]) {
    // Handles the case where no files are provided
    if (argc < 2) {
        fprintf(stderr, "No files provided!\n");
        return 1;
    }

    int fd[2];

    // Creates the pipe and handles the case where the pipe fails
    if (pipe(fd) == -1){
        fprintf(stderr, "Pipe Failed!\n");
        return 1;
    }

    int num_children = argc - 1;

    // Loops through the files and creates a child process for each file
    for (int i = 1; i < argc; i++){
        int pid = fork();
        if (pid == -1){
            fprintf(stderr, "Fork Failed!\n");
            return 2;
        }

        //Testing the order of the children
        //if (pid == 0 && i == 2) {
        //    sleep(1);
        //}
 

        // Child process
        if (pid == 0){
            // Closes the read end of the pipe because the child process does not need to read
            close(fd[READ]);
            calculations(argv[i], fd[WRITE]);
            // Closes the write end of the pipe
            close(fd[WRITE]);
            return 0;
        }
    }
    // Parent process

    // Closes the write end of the pipe because the parent process does not need to write
    close(fd[WRITE]);

    double pmin = DBL_MAX;
    double pmax = -DBL_MAX;

    // Waits for any child processes to finish
    while (num_children > 0){
        char filename[BUFFER_SIZE];
        double sum;
        double dif;
        double min;
        double max;

        // Waits for the child process to finish
        if (wait(NULL) == -1){
            fprintf(stderr, "Error waiting for child process!\n");
            return 3;
        } else {
            // Reads the sum, difference, min, max, and filename from the pipe
            read(fd[READ], &min, sizeof(double));
            read(fd[READ], &max, sizeof(double));
            read(fd[READ], &sum, sizeof(double));
            read(fd[READ], &dif, sizeof(double));
            read(fd[READ], &filename, sizeof(char) * BUFFER_SIZE);
            // Handles the case where there are no numbers in the file
            if (sum == DBL_MAX && dif == -DBL_MAX){
                fprintf(stdout, "%s SUM=NONE DIF=NONE MIN=NONE MAX=NONE\n", filename);
            } else {
                fprintf(stdout, "%s SUM=%lf DIF=%lf MIN=%lf MAX=%lf \n", filename, sum, dif, min, max);
            }
            // Removes 1 from the number of children left to wait for
            num_children--;

            // Updates the parent min and max
            if (min < pmin) pmin = min;
            
            if (max > pmax) pmax = max;
            
        }
    }

    // Closes the read end of the pipe
    close(fd[READ]);

    // Prints the parent min and max
    fprintf(stdout, "MINIMUM=%lf MAXIMUM=%lf\n", pmin, pmax);



    return 0;
}