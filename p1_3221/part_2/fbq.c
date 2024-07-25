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

// Global variables
process processes[MAX_PROCESSES + 1];
int numProcesses = 0;
int nextProcessIndex = 0;
process_queue q0, q1, q2, waitingQueue;
process *cpu[NUMBER_OF_PROCESSORS];
int totalWaitTime = 0;
int contextSwitchCount = 0;
int currentTime = 0;
int utilizedCpuTime = 0;

int Q0_QUANTUM = 8;
int Q1_QUANTUM = 20;

// Comparing by pid in case of timing conflicts
int compareByPID(const void *a, const void *b) {
    process *p1 = *((process **)a);
    process *p2 = *((process **)b);
    return (p1->pid < p2->pid) ? -1 : (p1->pid > p2->pid);
}

// Returns the ammount of running processes
int countRunningProcesses() {
    int count = 0;
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) count++;
    }
    return count;
}

// Returns the ammount of incoming processes
int countIncomingProcesses() {
    return numProcesses - nextProcessIndex;
}

// Returns a pointer to the next process in the ready queue
process *getNextProcess() {
    if (q0.size > 0) {
        process *p = q0.front->data;
        dequeueProcess(&q0);
        return p;
    }
    if (q1.size > 0) {
        process *p = q1.front->data;
        dequeueProcess(&q1);
        return p;
    }
    if (q2.size > 0) {
        process *p = q2.front->data;
        dequeueProcess(&q2);
        return p;
    }
    return NULL;
}

// Handles new arrivals by adding them to the ready queue
void handleNewArrivals() {
    while (nextProcessIndex < numProcesses && processes[nextProcessIndex].arrivalTime <= currentTime) {
        enqueueProcess(&q0, &processes[nextProcessIndex++]);
    }
}

// Handles IO completions by moving processes to the io queue
void handleIOCompletion() {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        if (p->bursts[p->currentBurst].step == p->bursts[p->currentBurst].length) {
            p->currentBurst++;
            enqueueProcess(&q0, p);
        } else {
            enqueueProcess(&waitingQueue, p);
        }
    }
}

// Handles CPU movement by sorting the temp queue and adding them to the ready queue
void handleCPUMovement() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] == NULL) {
            cpu[i] = getNextProcess();
        }
    }
}

// Handles process completions by moving them to the io queue or setting their end time
void handleProcessCompletion() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) {
            process *p = cpu[i];
            int currentBurst = p->currentBurst;
            if (p->bursts[currentBurst].step == p->bursts[currentBurst].length) {
                p->currentBurst++;
                if (p->currentBurst < p->numberOfBursts) {
                    enqueueProcess(&waitingQueue, p);
                } else {
                    p->endTime = currentTime;
                }
                cpu[i] = NULL;
            } else if (p->currentQueue == 0 && p->bursts[currentBurst].step == Q0_QUANTUM) {
                p->currentQueue = 1;
                enqueueProcess(&q1, p);
                cpu[i] = NULL;
                contextSwitchCount++;
            } else if (p->currentQueue == 1 && p->bursts[currentBurst].step == Q1_QUANTUM) {
                p->currentQueue = 2;
                enqueueProcess(&q2, p);
                cpu[i] = NULL;
                contextSwitchCount++;
            }
        }
    }
}

// Updates the IO processes
void updateIOProcesses() {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        p->bursts[p->currentBurst].step++;
        enqueueProcess(&waitingQueue, p);
    }
}

// Updates the ready processes
void updateReadyProcesses() {
    int size = q0.size;
    for (int i = 0; i < size; i++) {
        process *p = q0.front->data;
        dequeueProcess(&q0);
        p->waitingTime++;
        enqueueProcess(&q0, p);
    }
    size = q1.size;
    for (int i = 0; i < size; i++) {
        process *p = q1.front->data;
        dequeueProcess(&q1);
        p->waitingTime++;
        enqueueProcess(&q1, p);
    }
    size = q2.size;
    for (int i = 0; i < size; i++) {
        process *p = q2.front->data;
        dequeueProcess(&q2);
        p->waitingTime++;
        enqueueProcess(&q2, p);
    }
}

// Updates the CPU processes
void updateCPUProcesses() {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpu[i] != NULL) {
            cpu[i]->bursts[cpu[i]->currentBurst].step++;
        }
    }
}

// Calculates and prints the results
void calculateAndPrintResults() {
    int totalTurnaroundTime = 0;
    for (int i = 0; i < numProcesses; i++) {
        totalTurnaroundTime += processes[i].endTime - processes[i].arrivalTime;
        totalWaitTime += processes[i].waitingTime;
    }

    double averageWaitTime = totalWaitTime / (double)numProcesses;
    double averageTurnaroundTime = totalTurnaroundTime / (double)numProcesses;
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

// Runs the simulation
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

        if (countRunningProcesses() == 0 && countIncomingProcesses() == 0 && waitingQueue.size == 0) break;

        currentTime++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Q0_QUANTUM> <Q1_QUANTUM>\n", argv[0]);
        return -1;
    }

    Q0_QUANTUM = atoi(argv[1]);
    Q1_QUANTUM = atoi(argv[2]);

    // Initialize the CPU array
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cpu[i] = NULL;
    }

    initializeProcessQueue(&q0);
    initializeProcessQueue(&q1);
    initializeProcessQueue(&q2);
    initializeProcessQueue(&waitingQueue);

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