/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sch-helpers.h"

process processes[MAX_PROCESSES+1];
int numProcesses = 0;
int nextProcessIndex = 0;
process_queue readyQueue;
process_queue ioQueue;
process *cpu[NUMBER_OF_PROCESSORS];
int totalWaitTime = 0;
int contextSwitchCount = 0;
int currentTime = 0;
int utilizedCpuTime = 0;
process *tempQueue[MAX_PROCESSES];
int tempQueueSize = 0;
int timeQuantum;

int compareByPID(const void *a, const void *b) {
    process *p1 = *((process**)a);
    process *p2 = *((process**)b);
    return (p1->pid < p2->pid) ? -1 : (p1->pid > p2->pid);
}

int countRunningProcesses() {
    int count = 0;
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) count++;
    }
    return count;
}

int countIncomingProcesses() {
    return numProcesses - nextProcessIndex;
}

process *getNextProcess() {
    if (readyQueue.size == 0) return NULL;
    process *p = readyQueue.front->data;
    dequeueProcess(&readyQueue);
    return p;
}

void handleNewArrivals() {
    while (nextProcessIndex < numProcesses && processes[nextProcessIndex].arrivalTime <= currentTime) {
        tempQueue[tempQueueSize++] = &processes[nextProcessIndex++];
    }
}

void handleIOCompletion() {
    int size = ioQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = ioQueue.front->data;
        dequeueProcess(&ioQueue);
        if (p->bursts[p->currentBurst].step == p->bursts[p->currentBurst].length) {
            p->currentBurst++;
            tempQueue[tempQueueSize++] = p;
        } else {
            enqueueProcess(&ioQueue, p);
        }
    }
}

void handleCPUMovement() {
    qsort(tempQueue, tempQueueSize, sizeof(process*), compareByPID);
    for (int i = 0; i < tempQueueSize; i++) {
        enqueueProcess(&readyQueue, tempQueue[i]);
    }
    tempQueueSize = 0;

    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] == NULL) {
            cpu[i] = getNextProcess();
        }
    }
}

void handleProcessCompletion() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) {
            if (cpu[i]->bursts[cpu[i]->currentBurst].step == cpu[i]->bursts[cpu[i]->currentBurst].length) {
                cpu[i]->currentBurst++;
                if (cpu[i]->currentBurst < cpu[i]->numberOfBursts) {
                    enqueueProcess(&ioQueue, cpu[i]);
                } else {
                    cpu[i]->endTime = currentTime;
                }
                cpu[i] = NULL;
            } else if (cpu[i]->bursts[cpu[i]->currentBurst].step % timeQuantum == 0) {
                tempQueue[tempQueueSize++] = cpu[i];
                cpu[i] = NULL;
                contextSwitchCount++;
            }
        }
    }
}

void updateIOProcesses() {
    int size = ioQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = ioQueue.front->data;
        dequeueProcess(&ioQueue);
        p->bursts[p->currentBurst].step++;
        enqueueProcess(&ioQueue, p);
    }
}

void updateReadyProcesses() {
    int size = readyQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = readyQueue.front->data;
        dequeueProcess(&readyQueue);
        p->waitingTime++;
        enqueueProcess(&readyQueue, p);
    }
}

void updateCPUProcesses() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) {
            cpu[i]->bursts[cpu[i]->currentBurst].step++;
        }
    }
}

void calculateAndPrintResults() {
    int totalTurnaroundTime = 0;
    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundTime += processes[i].endTime - processes[i].arrivalTime;
        totalWaitTime += processes[i].waitingTime;
    }

    double averageWaitTime = totalWaitTime / (double) numProcesses;
    double averageTurnaroundTime = totalTurnaroundTime / (double) numProcesses;
    double averageCPUUtilization = 100.0 * utilizedCpuTime / currentTime;

    printf("Average waiting time                 : %.2f units\n", averageWaitTime);
    printf("Average turnaround time              : %.2f units\n", averageTurnaroundTime);
    printf("Time all processes finished          : %d\n", currentTime);
    printf("Average CPU utilization              : %.1f%%\n", averageCPUUtilization);
    printf("Number of context switches           : %d\n", contextSwitchCount);

    printf("PID(s) of last process(es) to finish :");
    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].endTime == currentTime) {
            printf(" %d", processes[i].pid);
        }
    }
    printf("\n");
}

void runSimulation() {
    while (1) {
        handleNewArrivals();
        handleProcessCompletion();
        handleIOCompletion();
        handleCPUMovement();
        updateIOProcesses();
        updateReadyProcesses();
        updateCPUProcesses();

        utilizedCpuTime += countRunningProcesses();

        if (countRunningProcesses() == 0 && countIncomingProcesses() == 0 && ioQueue.size == 0) break;

        currentTime++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <time_quantum>\n", argv[0]);
        return -1;
    }

    timeQuantum = atoi(argv[1]);

    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cpu[i] = NULL;
    }

    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&ioQueue);

    int readResult = 0;
    while ((readResult = readProcess(&processes[numProcesses])) != 0) {
        if (readResult == 1) numProcesses++;
        if (numProcesses > MAX_PROCESSES) break;
    }

    if (numProcesses == 0) {
        fprintf(stderr, "Error: no processes specified in input.\n");
        return -1;
    } else if (numProcesses > MAX_PROCESSES) {
        fprintf(stderr, "Error: too many processes specified in input; they cannot number more than %d.\n", MAX_PROCESSES);
        return -1;
    }

    qsort(processes, numProcesses, sizeof(process), compareByArrival);

    runSimulation();

    calculateAndPrintResults();

    return 0;
}