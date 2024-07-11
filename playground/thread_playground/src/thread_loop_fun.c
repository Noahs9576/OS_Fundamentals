#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int mails = 0;

// AVOID RACE CONDITION!!!!
pthread_mutex_t mutex;

void* routine() {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        mails++;
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[]) {
    pthread_t th[8];
    int i;
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < 8; i++) {
        if (pthread_create(&th[i], NULL, &routine, NULL) != 0) {
            return 1;
        }
        printf("Thread %d started...\n", i);
    }

    for (i = 0; i < 8; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            return 2;
        }
        printf("%d threads done!\n", i);
    }

    pthread_mutex_destroy(&mutex);
    printf("Mails sent: %d\n", mails);
    return 0;
}