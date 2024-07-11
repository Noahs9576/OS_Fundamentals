/*
Family Name: Silva
Given Name(s): Noah
Student Number: 220090890
EECS Login ID (the one you use to access the red server): noahs957
YorkU email address (the one that appears in eClass): noahs957@my.yorku.ca
*/

// Header files
#include <stdio.h>
#include <stdlib.h>

#include "sch-helpers.h"  

// Global variables
process processes[MAX_PROCESSES+1];   
int numberOfProcesses=0;              

 int main(int argc, char *argv[]) {

    // Variables needed for calculations or printing
    int status;
    int currentTime=0;
    int contextSwitches=0;
    int lastProcess=-1;

    double totalWaitTime=0;
    double totalTurnaroundTime=0;

    while (status=readProcess(&processes[numberOfProcesses]))  {
         if(status==1)  numberOfProcesses ++;
    } 

    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    // Loop through all processes
    for (int i = 0; i < numberOfProcesses; i++) {
        // Get the current process and set its pointer to the current process address
        process *p = &processes[i];
        int turnaroundTime = 0;

        // If the current time is less than the arrival time of the process set the current time to the arrival time
        if (currentTime < p->arrivalTime) {
            currentTime = p->arrivalTime;
        }

        // Set the start time and waiting time of the process
        p->startTime = currentTime;
        p->waitingTime = currentTime - p->arrivalTime;

        // Loop through all the bursts of the process
        for (int j = 0; j < p->numberOfBursts; j++) {
            currentTime += p->bursts[j].length;
        }

        // Set the end time of the process and calculate the turnaround time
        p->endTime = currentTime;
        turnaroundTime = p->endTime - p->arrivalTime;

        // Add the waiting time and turnaround time to the total waiting and turnaround times
        totalWaitTime += p->waitingTime;
        totalTurnaroundTime += turnaroundTime;

        // Store each process's pid in the last process variable so the last one overwrites the rest
        lastProcess = p->pid;

    }

    // Calculations
    double avgWaitTime = totalWaitTime / numberOfProcesses;
    double avgTurnaroundTime = totalTurnaroundTime / numberOfProcesses;
    double cpuUtilization = (double)currentTime / (currentTime + totalWaitTime) * 100;

    // Printing the necessary information
    fprintf(stdout, "Avg. Waiting Time: %.2f\n", avgWaitTime);
    fprintf(stdout, "Avg. Turnaround Time: %.2f\n", avgTurnaroundTime);
    fprintf(stdout, "Total Time: %d\n", currentTime);
    fprintf(stdout, "CPU Utilization: %.2f%% \n", cpuUtilization);
    fprintf(stdout, "Context Switches: %d\n", contextSwitches);
    fprintf(stdout, "Last PID: %d\n", lastProcess);

 }

