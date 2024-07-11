#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* routine() {
    printf("Hello from thread\n");
    sleep(3);
    printf("Thread is done\n");
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, &routine, NULL) != 0) {
        return 1;
    }

    if (pthread_create(&t2, NULL, &routine, NULL) != 0) {
        return 1;
    }

    // NEED TO WAIT FOR THREAD TO FINISH
    if (pthread_join(t1, NULL) != 0) {
        return 1;
    } 

    if (pthread_join(t2, NULL) != 0) {
        return 1;
    }

    return 0;
}