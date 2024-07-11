#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define THREADS 4

void* roll_dice() {
    int value = rand() % 6 + 1;
    int* result = malloc(sizeof(int));
    *result = value;
    printf("Dice rolled: %d\n", value);
    return (void*) result;
}

int min = -1;
int max = -1;

int main(int argc, char *argv[]) {
    int* result;
    srand(time(NULL));
    pthread_t th[THREADS];
    int i;
    for (i = 0; i < THREADS; i++) {
        if (pthread_create(&th[i], NULL, &roll_dice, NULL) != 0) {
            return 1;
        }
    }

    for (i = 0; i < THREADS; i++) {
        if (pthread_join(th[i], (void**) &result) != 0) {
            return 2;
        }

        if (min == -1 || *result < min) {
            min = *result;
        }

        if (max == -1 || *result > max) {
            max = *result;
        }
    }

    printf("Min: %d\n", min);
    printf("Max: %d\n", max);

    free(result);
    return 0;
}