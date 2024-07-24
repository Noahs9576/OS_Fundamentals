#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sch-helpers.h"

process processes[MAX_PROCESSES + 1];
int numberOfProcesses = 0;
int nextProcess = 0;
process_queue q0, q1, q2, waitingQueue;
process *cpus[NUMBER_OF_PROCESSORS];
int totalWaitingTime = 0;
int totalContextSwitches = 0;
int simulationTime = 0;
int cpuTimeUtilized = 0;

int Q0_QUANTUM = 8;
int Q1_QUANTUM = 20;

void initializeGlobals() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cpus[i] = NULL;
    }
    simulationTime = 0;
    cpuTimeUtilized = 0;
    totalWaitingTime = 0;
    totalContextSwitches = 0;
    numberOfProcesses = 0;
    nextProcess = 0;
    initializeProcessQueue(&q0);
    initializeProcessQueue(&q1);
    initializeProcessQueue(&q2);
    initializeProcessQueue(&waitingQueue);
}

int compareProcessPointers(const void *a, const void *b) {
    process *pa = *(process **)a;
    process *pb = *(process **)b;
    if (pa->pid < pb->pid) return -1;
    if (pa->pid > pb->pid) return 1;
    assert(0);
    return 0;
}

int runningProcesses() {
    int result = 0;
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) result++;
    }
    return result;
}

int incomingProcesses() {
    return numberOfProcesses - nextProcess;
}

process *nextScheduledProcess() {
    if (q0.size > 0) {
        process *result = q0.front->data;
        dequeueProcess(&q0);
        return result;
    }
    if (q1.size > 0) {
        process *result = q1.front->data;
        dequeueProcess(&q1);
        return result;
    }
    if (q2.size > 0) {
        process *result = q2.front->data;
        dequeueProcess(&q2);
        return result;
    }
    return NULL;
}

void moveIncomingProcesses() {
    while (nextProcess < numberOfProcesses && processes[nextProcess].arrivalTime <= simulationTime) {
        enqueueProcess(&q0, &processes[nextProcess++]);
    }
}

void moveWaitingProcesses() {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *front = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        if (front->bursts[front->currentBurst].step == front->bursts[front->currentBurst].length) {
            front->currentBurst++;
            enqueueProcess(&q0, front);
        } else {
            enqueueProcess(&waitingQueue, front);
        }
    }
}

void moveReadyProcesses() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] == NULL) {
            cpus[i] = nextScheduledProcess();
        }
    }
}

void moveRunningProcesses() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) {
            process *p = cpus[i];
            int currentBurst = p->currentBurst;
            if (p->bursts[currentBurst].step == p->bursts[currentBurst].length) {
                p->currentBurst++;
                if (p->currentBurst < p->numberOfBursts) {
                    enqueueProcess(&waitingQueue, p);
                } else {
                    p->endTime = simulationTime;
                }
                cpus[i] = NULL;
            } else if (p->currentQueue == 0 && p->bursts[currentBurst].step == Q0_QUANTUM) {
                p->currentQueue = 1;
                enqueueProcess(&q1, p);
                cpus[i] = NULL;
                totalContextSwitches++;
            } else if (p->currentQueue == 1 && p->bursts[currentBurst].step == Q1_QUANTUM) {
                p->currentQueue = 2;
                enqueueProcess(&q2, p);
                cpus[i] = NULL;
                totalContextSwitches++;
            }
        }
    }
}

void updateWaitingProcesses() {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *front = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        front->bursts[front->currentBurst].step++;
        enqueueProcess(&waitingQueue, front);
    }
}

void updateReadyProcesses() {
    int size = q1.size;
    for (int i = 0; i < size; i++) {
        process *front = q1.front->data;
        dequeueProcess(&q1);
        front->waitingTime++;
        enqueueProcess(&q1, front);
    }
    size = q2.size;
    for (int i = 0; i < size; i++) {
        process *front = q2.front->data;
        dequeueProcess(&q2);
        front->waitingTime++;
        enqueueProcess(&q2, front);
    }
}

void updateRunningProcesses() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) {
            cpus[i]->bursts[cpus[i]->currentBurst].step++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Q0_QUANTUM> <Q1_QUANTUM>\n", argv[0]);
        return -1;
    }

    Q0_QUANTUM = atoi(argv[1]);
    Q1_QUANTUM = atoi(argv[2]);

    int sumOfTurnaroundTimes = 0;
    int doneReading = 0;

    initializeGlobals();
    while (doneReading = readProcess(&processes[numberOfProcesses])) {
        if (doneReading == 1) numberOfProcesses++;
        if (numberOfProcesses > MAX_PROCESSES) break;
    }

    if (numberOfProcesses == 0) {
        fprintf(stderr, "Error: no processes specified in input.\n");
        return -1;
    } else if (numberOfProcesses > MAX_PROCESSES) {
        fprintf(stderr, "Error: too many processes specified in input; they cannot number more than %d.\n", MAX_PROCESSES);
        return -1;
    }

    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    while (1) {
        moveIncomingProcesses();
        moveRunningProcesses();
        moveWaitingProcesses();
        moveReadyProcesses();
        updateWaitingProcesses();
        updateReadyProcesses();
        updateRunningProcesses();
        cpuTimeUtilized += runningProcesses();
        if (runningProcesses() == 0 && incomingProcesses() == 0 && waitingQueue.size == 0) break;
        simulationTime++;
    }

    for (int i = 0; i < numberOfProcesses; i++) {
        sumOfTurnaroundTimes += processes[i].endTime - processes[i].arrivalTime;
        totalWaitingTime += processes[i].waitingTime;
    }

    printf("Average waiting time : %.2f units\n"
           "Average turnaround time : %.2f units\n"
           "Time all processes finished : %d\n"
           "Average CPU utilization : %.1f%%\n"
           "Number of context switches : %d\n",
           totalWaitingTime / (double) numberOfProcesses,
           sumOfTurnaroundTimes / (double) numberOfProcesses,
           simulationTime,
           100.0 * cpuTimeUtilized / simulationTime,
           totalContextSwitches);

    printf("PID(s) of last process(es) to finish :");
    for (int i = 0; i < numberOfProcesses; i++) {
        if (processes[i].endTime == simulationTime) {
            printf(" %d", processes[i].pid);
        }
    }
    printf("\n");
    return 0;
}