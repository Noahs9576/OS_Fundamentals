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
// Temp queue to hold processes moving to the ready queue
process *tempQueue[MAX_PROCESSES];
int tempQueueSize = 0;
int timeQuantum;

// Comparing by pid in case of timing conflicts
int compareByPID(const void *a, const void *b) {
    process *p1 = *((process**)a);
    process *p2 = *((process**)b);
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
    if (readyQueue.size == 0) return NULL;
    process *p = readyQueue.front->data;
    dequeueProcess(&readyQueue);
    return p;
}

// Handles new arrivals by adding them to the ready queue
void handleNewArrivals() {
    while (nextProcessIndex < numProcesses && processes[nextProcessIndex].arrivalTime <= currentTime) {
        tempQueue[tempQueueSize++] = &processes[nextProcessIndex++];
    }
}

// Handles IO completions by moving processes to the io queue
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

// Handles CPU movement by sorting the temp queue and adding them to the ready queue
void handleCPUMovement() {
    // Sort the temp queue by PID
    qsort(tempQueue, tempQueueSize, sizeof(process*), compareByPID);
    for (int i = 0; i < tempQueueSize; i++) {
        enqueueProcess(&readyQueue, tempQueue[i]);
    }
    tempQueueSize = 0;

    // Fill empty CPUs
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
            // If the process has completed all of its bursts then set its end time
            if (cpu[i]->bursts[cpu[i]->currentBurst].step == cpu[i]->bursts[cpu[i]->currentBurst].length) {
                cpu[i]->currentBurst++;
                // If the process has more bursts then move it to the io queue
                if (cpu[i]->currentBurst < cpu[i]->numberOfBursts) {
                    enqueueProcess(&ioQueue, cpu[i]);
                } else {
                    cpu[i]->endTime = currentTime;
                }
                cpu[i] = NULL;
            // If the process has completed the time quantum then move it to the temp queue
            } else if (cpu[i]->bursts[cpu[i]->currentBurst].step % timeQuantum == 0) {
                tempQueue[tempQueueSize++] = cpu[i];
                cpu[i] = NULL;
                contextSwitchCount++;
            }
        }
    }
}

// Updates the IO processes
void updateIOProcesses() {
    int size = ioQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = ioQueue.front->data;
        dequeueProcess(&ioQueue);
        p->bursts[p->currentBurst].step++;
        enqueueProcess(&ioQueue, p);
    }
}

// Updates the ready processes
void updateReadyProcesses() {
    int size = readyQueue.size;
    for (int i = 0; i < size; i++) {
        process *p = readyQueue.front->data;
        dequeueProcess(&readyQueue);
        p->waitingTime++;
        enqueueProcess(&readyQueue, p);
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

        if (countRunningProcesses() == 0 && countIncomingProcesses() == 0 && ioQueue.size == 0) break;

        currentTime++;
    }
}

int main(int argc, char *argv[]) {
    // Check for correct number of inputs
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <time_quantum>\n", argv[0]);
        return -1;
    }

    timeQuantum = atoi(argv[1]);

    // Initialize the CPU array
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cpu[i] = NULL;
    }

    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&ioQueue);

    int readResult = 0;
    // Read the processes
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