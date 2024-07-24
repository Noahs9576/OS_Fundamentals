/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sch-helpers.h"

// Global variables
process processes[MAX_PROCESSES + 1]; // Array to hold all processes
int numberOfProcesses = 0; // Total number of processes
int nextProcess = 0; // Index of next process to arrive
process_queue readyQueue; // Ready queue to hold all ready processes
process_queue waitingQueue; // Waiting queue to hold all processes in I/O waiting
process *cpus[NUMBER_OF_PROCESSORS]; // Processes running on each CPU
int totalWaitingTime = 0; // Total time processes spent waiting
int totalContextSwitches = 0; // Total number of preemptions
int simulationTime = 0; // Time steps simulated
int cpuTimeUtilized = 0; // Time steps each CPU was executing
process *preReadyQueue[MAX_PROCESSES]; // Holds processes moving to the ready queue this timestep (for sorting)
int preReadyQueueSize = 0;

// Function to initialize all global variables
void initializeGlobals(void) {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cpus[i] = NULL;
    }
    simulationTime = 0;
    cpuTimeUtilized = 0;
    totalWaitingTime = 0;
    totalContextSwitches = 0;
    numberOfProcesses = 0;
    nextProcess = 0;
    preReadyQueueSize = 0;
    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
}

// Function to compare process pointers by process ID
int compareProcessPointers(const void *a, const void *b) {
    process *pa = *(process **)a;
    process *pb = *(process **)b;
    if (pa->pid < pb->pid) return -1;
    if (pa->pid > pb->pid) return 1;
    assert(0); // Should never happen
    return 0;
}

// Function to return the number of running processes
int runningProcesses(void) {
    int result = 0;
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) result++;
    }
    return result;
}

// Function to return the number of incoming processes
int incomingProcesses(void) {
    return numberOfProcesses - nextProcess;
}

// Function to fetch and dequeue the next scheduled process from the ready queue
process *nextScheduledProcess(void) {
    if (readyQueue.size == 0) return NULL;
    process *result = readyQueue.front->data;
    dequeueProcess(&readyQueue);
    return result;
}

// Function to enqueue newly arriving processes in the ready queue
void moveIncomingProcesses(void) {
    while (nextProcess < numberOfProcesses && processes[nextProcess].arrivalTime <= simulationTime) {
        preReadyQueue[preReadyQueueSize++] = &processes[nextProcess++];
    }
}

// Function to move waiting processes that are finished their I/O bursts to ready
void moveWaitingProcesses(void) {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *front = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        assert(front->bursts[front->currentBurst].step <= front->bursts[front->currentBurst].length);
        if (front->bursts[front->currentBurst].step == front->bursts[front->currentBurst].length) {
            front->currentBurst++;
            preReadyQueue[preReadyQueueSize++] = front;
        } else {
            enqueueProcess(&waitingQueue, front);
        }
    }
}

// Function to move ready processes into free CPUs according to scheduling algorithm
void moveReadyProcesses(void) {
    qsort(preReadyQueue, preReadyQueueSize, sizeof(process*), compareProcessPointers);
    for (int i = 0; i < preReadyQueueSize; i++) {
        enqueueProcess(&readyQueue, preReadyQueue[i]);
    }
    preReadyQueueSize = 0;
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] == NULL) {
            cpus[i] = nextScheduledProcess();
        }
    }
}

// Function to move running processes that have finished their CPU bursts to waiting, or terminate them
void moveRunningProcesses(void) {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) {
            if (cpus[i]->bursts[cpus[i]->currentBurst].step == cpus[i]->bursts[cpus[i]->currentBurst].length) {
                cpus[i]->currentBurst++;
                if (cpus[i]->currentBurst < cpus[i]->numberOfBursts) {
                    enqueueProcess(&waitingQueue, cpus[i]);
                } else {
                    cpus[i]->endTime = simulationTime;
                }
                cpus[i] = NULL;
            }
        }
    }
}

// Function to increment each waiting process' current I/O burst's progress
void updateWaitingProcesses(void) {
    int size = waitingQueue.size;
    for (int i = 0; i < size; i++) {
        process *front = waitingQueue.front->data;
        dequeueProcess(&waitingQueue);
        front->bursts[front->currentBurst].step++;
        enqueueProcess(&waitingQueue, front);
    }
}

// Function to increment waiting time for each process in the ready queue
void updateReadyProcesses(void) {
    for (int i = 0; i < readyQueue.size; i++) {
        process *front = readyQueue.front->data;
        dequeueProcess(&readyQueue);
        front->waitingTime++;
        enqueueProcess(&readyQueue, front);
    }
}

// Function to update the progress for all currently executing processes
void updateRunningProcesses(void) {
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        if (cpus[i] != NULL) {
            cpus[i]->bursts[cpus[i]->currentBurst].step++;
        }
    }
}

int main(void) {
    int sumOfTurnaroundTimes = 0;
    int doneReading = 0;
    int i;

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

    for (i = 0; i < numberOfProcesses; i++) {
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
    for (i = 0; i < numberOfProcesses; i++) {
        if (processes[i].endTime == simulationTime) {
            printf(" %d", processes[i].pid);
        }
    }
    printf("\n");
    return 0;
}