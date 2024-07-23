/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

//Header files
#include <stdio.h>
#include <stdlib.h>

#include "sch-helpers.h"

// Global variables
process processes[MAX_PROCESSES + 1];
int numberOfProcesses = 0;
int contextSwitches = 0;

// Function to simulate FCFS scheduling
void fcfs(int cpuIndex, int* currentTime, int* cpuFreeUntil, double* totalWaitTime, double* totalTurnaroundTime, int* lastProcess) {
    for (int i = 0; i < numberOfProcesses; i++) {
        // Get the current process address and set to process pointer
        process *p = &processes[i];
        int turnaroundTime = 0;

        // If the current time is less than the arrival time of the process, set the current time to the arrival time
        if (*currentTime < p->arrivalTime) {
            *currentTime = p->arrivalTime;
        }

        // If the selected CPU is busy, update the current time to when the CPU will be free
        if (*currentTime < cpuFreeUntil[cpuIndex]) {
            *currentTime = cpuFreeUntil[cpuIndex];
        }

        // Set the start time and waiting time of the process
        p->startTime = *currentTime;
        p->waitingTime = *currentTime - p->arrivalTime;

        // Loop through all the bursts of the process
        for (int j = 0; j < p->numberOfBursts; j++) {
            *currentTime += p->bursts[j].length;
        }

        // Set the end time of the process and calculate the turnaround time
        p->endTime = *currentTime;
        turnaroundTime = p->endTime - p->arrivalTime;

        // Update the CPU free time
        cpuFreeUntil[cpuIndex] = *currentTime;

        // Add the waiting time and turnaround time to the total waiting and turnaround times
        *totalWaitTime += p->waitingTime;
        *totalTurnaroundTime += turnaroundTime;

        // Store each process's pid in the last process variable so the last one overwrites the rest
        if (i == numberOfProcesses - 1) {
            *lastProcess = p->pid;
        }
    }
}

int main(int argc, char *argv[]) {
    // Variables needed for calculations or printing
    int currentTime = 0;
    int lastProcess = -1;

    double totalWaitTime = 0;
    double totalTurnaroundTime = 0;

    // Read processes from input
    while (readProcess(&processes[numberOfProcesses])) {
        numberOfProcesses++;
    }

    // Sort processes by arrival time
    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    // Initialize CPU array to track when each CPU will be free
    int cpuFreeUntil[NUMBER_OF_PROCESSORS] = {0};

    // Manage CPU scheduling
    for (int i = 0; i < numberOfProcesses; i++) {
        // Get the current process address and set to process pointer
        process *p = &processes[i];

        // Find the first available CPU
        int selectedCPU = 0;
        for (int j = 1; j < NUMBER_OF_PROCESSORS; j++) {
            if (cpuFreeUntil[j] < cpuFreeUntil[selectedCPU]) {
                selectedCPU = j;
            }
        }

        // Call fcfs function for the selected CPU
        fcfs(selectedCPU, &currentTime, cpuFreeUntil, &totalWaitTime, &totalTurnaroundTime, &lastProcess);
    }

    // Calculations
    double avgWaitTime = totalWaitTime / numberOfProcesses;
    double avgTurnaroundTime = totalTurnaroundTime / numberOfProcesses;
    double totalBusyTime = 0;

    // Loop through all the CPUs and calculate the total busy time
    for (int i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        totalBusyTime += cpuFreeUntil[i];
    }
    double cpuUtilization = (totalBusyTime / (NUMBER_OF_PROCESSORS * currentTime)) * 100;

    // Printing the necessary information
    fprintf(stdout, "Avg. Waiting Time: %.2f\n", avgWaitTime);
    fprintf(stdout, "Avg. Turnaround Time: %.2f\n", avgTurnaroundTime);
    fprintf(stdout, "Total Time: %d\n", currentTime);
    fprintf(stdout, "CPU Utilization: %.2f%%\n", cpuUtilization);
    fprintf(stdout, "Context Switches: %d\n", contextSwitches);
    fprintf(stdout, "Last PID: %d\n", lastProcess);

    return 0;
}
