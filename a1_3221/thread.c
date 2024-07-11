/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

// Header Files
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <float.h> 

// Struct for thread responses
typedef struct {
    const char *filename;
    double sum;
    double dif;
    double min;
    double max;
} response;

// Global variables for min and max 
double g_min, g_max;

// Function that reads in a file and calculates the sum, difference, min, and max then passes it to the response struct then exits
void* calculations(void *param) {
    // Casts the param to a response struct
    response *res = (response*)param;

    // Opens the file from the struct
    FILE *file = fopen(res->filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No file found named: %s\n", res->filename);
        exit(1);
    }
    
    double num;
    res->min = DBL_MAX;
    res->max = -DBL_MAX;

    int cnt = 0;
    // Reads in the file and calculates the sum, difference, min, and max
    while (fscanf(file, "%lf", &num) != EOF) {
        if (num < res->min) res->min = num;
        if (num > res->max) res->max = num;
        cnt++;
    }
    fclose(file);

    // Handles the case where there are no numbers in the file
    if (cnt == 0) {
        res->min = DBL_MAX;
        res->max = -DBL_MAX;
        res->sum = DBL_MAX;
        res->dif = -DBL_MAX;

        pthread_exit(res);
    } else {
        res->sum = res->min + res->max;
        res->dif = res->min - res->max;

        if (res->min < g_min) g_min = res->min;
        if (res->max > g_max) g_max = res->max;

        pthread_exit(res);
    }


}



int main(int argc, char *argv[]) {
    // Checks if there are no files provided
    if (argc < 2) {
        fprintf(stderr, "No files provided!\n");
        return 1;
    }

    g_min = DBL_MAX;
    g_max = -DBL_MAX;

    int num_threads = argc - 1;

    // Creates the threads and response structs
    pthread_t th[num_threads];
    response **res = malloc(sizeof(response *) * num_threads);

    // Loops through the files and creates a thread for each file
    for (int i = 0; i < num_threads; i++) {
        res[i] = malloc(sizeof(response));
        res[i]->filename = argv[i+1];
        if (pthread_create(&th[i], NULL, &calculations, res[i]) != 0) {
            fprintf(stderr, "Thread creation failed!\n");
            return 2;
        }
    }

    // Gets the results from the threads and prints them
    for (int i = 0; i < num_threads; i++) {
        response *result;
        // Waits for the thread to finish
        if (pthread_join(th[i], (void**) &result) != 0) {
            fprintf(stderr, "Thread join failed!\n");
            return 3;
        }
        
        // Handles the case where there are no numbers in the file
        if (result->min == DBL_MAX && result->max == -DBL_MAX) {
            fprintf(stdout, "%s SUM=NONE DIF=NONE MIN=NONE MAX=NONE\n", result->filename);
            free(result);
            continue;
        }
        
        // Prints the results
        fprintf(stdout, "%s SUM=%lf DIF=%lf MIN=%lf MAX=%lf\n", result->filename, result->sum, result->dif, result->min, result->max);
        free(result);
    }

    fprintf(stdout, "MINIMUM=%lf MAXIMUM=%lf\n", g_min, g_max);

    // Frees the response structs
    free(res);

    return 0;
}